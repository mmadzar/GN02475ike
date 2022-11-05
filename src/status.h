#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"

class Status : public StatusBase
{
public:
  byte busBytes[2048];
  int busBytesSize = 0;
  String ibusMessage;
  uint8_t ibusSend[0xFF] = {0x00};
  long counts = 0;
  int ledMode = 0; // 0 off, 1 on, 2 blink, 3 fade, 4 min, 5 max

  int consumption = 1; // cycling interval value for counter 0-65535
  int rpm = 600;       // 600 minimum value to keep DSC working
  int coolant_temp = 90;
  double temp_sensor = 0;
  int digipot1 = 0;
  int digipot2 = 0;
  double litres1 = 0;
  double litres2 = 0;
  double ohm_per_liter1 = 10;
  double ohm_per_liter2 = 10;
  int ikeFuelLevel = -1; // used for pot calibration, received from can bus

  char *midVerb1 = "midVerb1";
  char *midVerb2 = "midVerb2";
  char *midReichw = "midReichw";
  char *ikeVerb1 = "ikeVerb1 ";
  char *ikeVerb2 = "ikeVerb2 ";
  char *ikeReichw = "ikeReichw";

  int sensors[SensorCount];

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();
    // root["vacuum_min"] = brakesSettings.vacuum_min;
    // root["vacuum_max"] = brakesSettings.vacuum_max;

    // TODO add display panel matrix data

    JsonObject jsensors = root.createNestedObject("sensors");
    for (size_t i = 0; i < SensorCount; i++)
    {
      jsensors[pinsSettings.sensors[i].name] = sensors[i];
    }

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
