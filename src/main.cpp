#include <Arduino.h>
#include <esp_wifi.h>
#include "appconfig.h"
#include "status.h"
#include "shared/WiFiOTA.h"
#include "shared/MqttPubSub.h"
#include "IBus.h"
#include "shared/Bytes2WiFi.h"
#include "DigiPot.h"
#include "Sensors.h"

Status status;
Settings settings;
Intervals intervals;
WiFiSettings wifiSettings;
WiFiOTA wota;
IBus ibus;
MqttPubSub mqtt;
Bytes2WiFi portSavvy;
Bytes2WiFi portBytes;
Bytes2WiFi portDebug;
DigiPot dpot;
Sensors sensors;

long loops = 0;
long lastLoopReport = 0;

bool firstRun = true;

int calibrationCounter = 0;
int lastCalibrateCommand = 0; // miliseconds when last calibrate command was sent
int lastValueFromIKE = 0;
bool calibrationInProgress = false;
int potToCalibrate = -1;
int potInCalibration = -1;
int currentPos = -1;
int targetLitres = 0;
int referentPotLitres = 0; // value that is set by referent pot. need to take away for targeting value. displayed from IKE when referent pot set to referent point.
// inverter PWR
bool lastInverterPWR = false;

void setup()
{
  Serial.begin(115200);
  pinMode(settings.led, OUTPUT);
  wota.setupWiFi();
  wota.setupOTA();
  ibus.setup(portSavvy, portBytes, portDebug);
  mqtt.setup();
  sensors.setup(mqtt);
  portSavvy.setup(23);
  portBytes.setup(24);
  portDebug.setup(25);
  dpot.setup();
}

void loop()
{
  status.currentMillis = millis();
  wota.handleWiFi();
  wota.handleOTA();
  if (!firstRun && loops % 10 == 0)
  {
    mqtt.handle();
    dpot.handle();
  }

  // handle inverter pwr
  if (status.inverterPWR != lastInverterPWR)
  {
    lastInverterPWR = status.inverterPWR;
    mqtt.sendMessageToTopic("GN02475acc/in/msft1", (lastInverterPWR == true ? String(1) : String(0)));
  }

  ibus.handle();
  sensors.handle();
  portSavvy.handle();
  portBytes.handle();
  portDebug.handle();

  if (!firstRun)
    mqtt.publishStatus(true);

  if (firstRun)
    firstRun = false;

  if (status.currentMillis - lastLoopReport > 1000) // number of loops in 1 second - for performance measurement
  {
    lastLoopReport = status.currentMillis;
    status.loops = loops;
    Serial.printf("Loops in a second %u\n", loops);
    loops = 0;

    // report on IKE display
  }
  loops++;
}
