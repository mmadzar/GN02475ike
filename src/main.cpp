#include <Arduino.h>
#include "shared/WiFiOTA.h"
#include "MqttMessageHandler.h"
#include "appconfig.h"
#include "status.h"
#include "shared/MqttPubSub.h"
#include <string.h>
#include "shared/Bytes2WiFi.h"
#include "IBus.h"
#include "CanBus.h"
#include "HC595165.h"

Status status;
Settings pins;
Intervals intervals;
WiFiSettings wifiSettings;
WiFiOTA wota;
MqttPubSub mqtt;
Bytes2WiFi bytesWiFi;
Bytes2WiFi portSavvy;
Bytes2WiFi portSavvyCan;
IBus ibus;
HC595165 hc595165;
CanBus can;

long loops = 0;
long lastLoopReport = 0;

bool firstRun = true;

void setup()
{
  SETTINGS.loadSettings();
  hc595165.setup(mqtt);

  pinMode(pins.led, OUTPUT);
  // Serial.begin(115200);
  // Serial.println("Serial started!");
  wota.setupWiFi();
  wota.setupOTA();
  mqtt.setup();
  portSavvy.setup(24);
  bytesWiFi.setup(25);
  ibus.setup(mqtt, portSavvy, bytesWiFi);

  portSavvyCan.setup(23);
  can.setup(mqtt, portSavvyCan);
}

void loop()
{
  status.currentMillis = millis();
  if (status.currentMillis - lastLoopReport > 1000) // number of loops in 1 second - for performance measurement
  {
    lastLoopReport = status.currentMillis;
    // Serial.print("Loops in a second: ");
    // Serial.println(loops);
    status.loops = loops;
    loops = 0;
    if (status.timeinfo.tm_year == 70)
    {
      getLocalTime(&(status.timeinfo), 10);
      if (status.timeinfo.tm_year != 70)
      {
        strftime(status.upsince, sizeof(status.upsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
        strftime(status.connectedsince, sizeof(status.connectedsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
      }
    }
  }
  else
  {
    loops++;
  }

  ibus.handle();
  hc595165.handle();

  wota.handleWiFi();
  wota.handleOTA();
  if (loops % 10 == 0) // check mqtt every 5th cycle
    mqtt.handle();

  can.handle();

  bytesWiFi.handle();
  portSavvyCan.handle();
  portSavvy.handle();

  mqtt.publishStatus(!firstRun);
  firstRun = false;
}
