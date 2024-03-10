#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475ike"

// relays
#define RELAY1 "relay1"
#define RELAY2 "relay2"
#define RELAY3 "relay3"
#define RELAY4 "relay4"
#define RELAY5 "relay5"
#define RELAY6 "relay6"
#define RELAY7 "relay7"
#define RELAY8 "relay8"

// sensors
#define FACTORY_BTN "factory_btn"
#define INPUT1 "input1"
#define INPUT2 "input2"
#define INPUT3 "input3"
#define INPUT4 "input4"
#define INPUT5 "input5"
#define INPUT6 "input6"
#define INPUT7 "input7"
#define INPUT8 "input8"

#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct Settings
{
#define ListenChannelsCount 0
  const char *listenChannels[ListenChannelsCount] = {};

  const gpio_num_t led = (gpio_num_t)2;     // status led
  const gpio_num_t can0_rx = (gpio_num_t)4; // can0 transciever rx line
  const gpio_num_t can0_tx = (gpio_num_t)5; // can0 transciever tx line

  const gpio_num_t serialRx = (gpio_num_t)3; // bus serial rx line - coded in ibustrx referenced lib
  const gpio_num_t serialTx = (gpio_num_t)1; // bus serial tx line - coded in ibustrx referenced lib

  uint8_t sdi = 13;
  uint8_t clk = 14;
  uint8_t rck = 15;
  uint8_t pl = 32;
  uint8_t q7 = 16;

#define SwitchCount 8
  SwitchConfig switches[SwitchCount] = {
      {devicet::relay, RELAY1, 0, switcht::on_off},
      {devicet::relay, RELAY2, 1, switcht::on_off},
      {devicet::relay, RELAY3, 2, switcht::on_off},
      {devicet::relay, RELAY4, 3, switcht::on_off},
      {devicet::relay, RELAY5, 4, switcht::on_off},
      {devicet::relay, RELAY6, 5, switcht::on_off},
      {devicet::relay, RELAY7, 6, switcht::on_off},
      {devicet::relay, RELAY8, 7, switcht::on_off}};

#define SensorCount 9
  SensorConfig sensors[SensorCount] = {
      {devicet::input, INPUT1, 0, sensort::digital},
      {devicet::input, INPUT2, 1, sensort::digital},
      {devicet::input, INPUT3, 2, sensort::digital},
      {devicet::input, INPUT4, 3, sensort::digital},
      {devicet::input, INPUT5, 4, sensort::digital},
      {devicet::input, INPUT6, 5, sensort::digital},
      {devicet::input, INPUT7, 6, sensort::digital},
      {devicet::input, INPUT8, 7, sensort::digital},
      {devicet::factory_btn, FACTORY_BTN, 34, sensort::digital}};

  int getSensorIndex(const char *name)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (strcmp(sensors[i].name, name) == 0)
        return i;
    }
  }

  int getSensorIndex(devicet device)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (sensors[i].device == device)
        return i;
    }
  }

  int getSwitchIndex(const char *name)
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (switches[i].name == name)
        return i;
    }
  }

  int getSwitchIndex(devicet device)
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (switches[i].device == device)
        return i;
    }
  }
};

struct Intervals
{
  int statusPublish = 1000;   // interval at which status is published to MQTT
  int Ibus2Mqtt = 50;         // send I-Bus messages to MQTT every n milliseconds. Accumulate messages until. Set this to 0 for forwarding all I-Bus messages to MQTT as they are received.
  int Can2Mqtt = 20;          // send CAN messages to MQTT every n milliseconds. Accumulate messages until. Set this to 0 for forwarding all CAN messages to MQTT as they are received.
  int click_onceDelay = 1000; // milliseconds
};

extern Settings settings;
extern Intervals intervals;

#endif /* APPCONFIG_H_ */
