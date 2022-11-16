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
PinsSettings pinsSettings;
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

void calibratePot()
{
  int fuelCheck = status.ikeFuelLevel;
  if (fuelCheck >= 128)
  {
    fuelCheck = fuelCheck - 128;
  }
  if (lastValueFromIKE == fuelCheck && status.currentMillis - lastCalibrateCommand > 2000) // execute calibration command every second
  {
    // set oposite pot value to 10
    if (potInCalibration != potToCalibrate)
    {
      if (currentPos == -1) // start testing
      {
        // reset positions before calibration
        mqtt.sendMesssageToTopic("ESP32carbustest/in/litres1", "0");
        mqtt.sendMesssageToTopic("ESP32carbustest/in/litres2", "0");

        targetLitres = 10;
        currentPos = 30;
      }
      if (fuelCheck != targetLitres)
      {
        Serial.print(currentPos);
        Serial.print(" -> ");
        Serial.print(status.ikeFuelLevel);
        Serial.print(" -> ");
        Serial.println(fuelCheck);
        // expected to set on 10.00 litres - start from calculated 8.5 and test every
        currentPos++;
        if (potInCalibration == 0)
          mqtt.sendMesssageToTopic("ESP32carbustest/in/litres1", String(dpot.resVal[currentPos][potInCalibration + 3] / 10.00, 1));
        else
          mqtt.sendMesssageToTopic("ESP32carbustest/in/litres2", String(dpot.resVal[currentPos][potInCalibration + 3] / 10.00, 1));
        lastCalibrateCommand = status.currentMillis;
      }
      else // targeted value
      {
        Serial.println("Referent pot set up");
        referentPotLitres = targetLitres;
        // referent pot setup done... move to the calibration of requested pot
        potInCalibration = potToCalibrate;
        targetLitres = 0;
        currentPos = -1;
      }
      if (currentPos > 358)
        currentPos = -1;
    }
    // calibrate requested pot
    else
    {
      if (currentPos == -1) // start testing
      {
        targetLitres = 0;
        currentPos = 0;
      }

      if (fuelCheck - referentPotLitres != targetLitres)
      {
        Serial.print("Pos: ");
        Serial.print(currentPos);
        Serial.print(" IKE: ");
        Serial.print(status.ikeFuelLevel);
        Serial.print(" chk: ");
        Serial.print(fuelCheck);
        Serial.print(" target: ");
        Serial.println(targetLitres);
        if (potInCalibration == 0)
          mqtt.sendMesssageToTopic("ESP32carbustest/in/litres1", String(dpot.resVal[currentPos][potInCalibration + 3] / 10.00, 1));
        else
          mqtt.sendMesssageToTopic("ESP32carbustest/in/litres2", String(dpot.resVal[currentPos][potInCalibration + 3] / 10.00, 1));
        lastCalibrateCommand = status.currentMillis;
        dpot.resVal[currentPos][potInCalibration + 5] = fuelCheck - referentPotLitres; // map current IKE value
      }
      else // targeted value
      {
        dpot.resVal[currentPos][potInCalibration + 5] = targetLitres;
        Serial.println("target found.");
        targetLitres++;
      }
      currentPos++;
      if (currentPos > 358)
      {
        currentPos = -1;
        calibrationInProgress = false;
        for (size_t i = 0; i < 358; i++)
        {
          Serial.print(i);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][0]);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][1]);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][2]);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][3] / 10.00);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][4] / 10.00);
          Serial.print("\t");
          Serial.print(dpot.resVal[i][5]);
          Serial.print("\t");
          Serial.println(dpot.resVal[i][6]);
          if (potInCalibration == 0)
            targetLitres = -100; // continue  with next pot
        }
      }
    }
  }
  lastValueFromIKE = fuelCheck;
}

void calibrate(int potNo /* 0 or 1 */)
{
  potToCalibrate = potNo;
  potInCalibration = abs(potNo - 1);
  calibrationInProgress = true;
  calibrationCounter = 0;
  calibratePot();
}

void setup()
{
  Serial.begin(115200);
  pinMode(pinsSettings.led, OUTPUT);
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

    //report on IKE display
  }
  loops++;
}
