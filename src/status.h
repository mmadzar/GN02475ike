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

  int digipot1 = 0;
  int digipot2 = 0;
  double litres1 = 0;
  double litres2 = 0;
  double ohm_per_liter1 = 10;
  double ohm_per_liter2 = 10;
  int ikeFuelLevel = -1; // used for pot calibration, received from can bus

  char *midVerb1 = "midVerb1 ";
  char *midVerb2 = "midVerb2 ";
  char *midReichw = "midReichw";
  char *ikeVerb1 = "ikeVerb1 ";
  char *ikeVerb2 = "ikeVerb2 ";
  char *ikeReichw = "ikeReichw";

  char ikeDisplay[21]; //extra char for strings

  int sensors[SensorCount];

  JsonObject GenerateJson()
  {
    JsonObject root = this->PrepareRoot();
    root["counts"] = counts;

    // TODO add display panel matrix data
    JsonObject jdisplay = root.createNestedObject("display");
    jdisplay["litres1"] = litres1;
    jdisplay["litres2"] = litres2;
    jdisplay["digipot1"] = digipot1;
    jdisplay["digipot2"] = digipot2;
    jdisplay["ohmPerLiter1"] = ohm_per_liter1;
    jdisplay["ohmPerLiter2"] = ohm_per_liter2;

    jdisplay["ikeDisplay"] = ikeDisplay;
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
