#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"

struct Status : public StatusBase
{
  bool reverse_lights = false;
  String ibusMessage;
  uint8_t ibusSend[0xFF] = {0x00};
  char ike_display[25]; // extra char for strings
  uint16_t maxBufferSizeIbus = 0;
  bool ibus2mqtt_enabled = true;
  bool can2mqtt_enabled = true;

  int sensors[SensorCount]{
      -1, -1, -1, -1, -1, -1, -1, -1, -1};
  int switches[SwitchCount]{
      0, 0, 0, 0, 0, 0, 0, 0};

  JsonObject GenerateJson()
  {
    JsonObject root = this->PrepareRoot();

    root["reverse_lights"] = reverse_lights;
    root["ike_display"] = ike_display;
    root["ibus2mqtt"] = intervals.Ibus2Mqtt;
    root["ibus2mqtt_enabled"] = ibus2mqtt_enabled;
    root["can2mqtt"] = intervals.Can2Mqtt;
    root["can2mqtt_enabled"] = can2mqtt_enabled;

    JsonObject jsensors = root.createNestedObject("sensors");
    for (size_t i = 0; i < SensorCount; i++)
      jsensors[settings.sensors[i].name] = sensors[i];

    JsonObject jswitches = root.createNestedObject("switches");
    for (size_t i = 0; i < SwitchCount; i++)
      jswitches[settings.switches[i].name] = switches[i];

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
