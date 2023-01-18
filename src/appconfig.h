#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475ike"

#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

#define POWER_12 "power12"
#define VOLTAGE_12 "voltage12"
#define POWER_HV "powerHV"
#define VOLTAGE_HV "voltageHV"

struct Settings
{
  const gpio_num_t led = (gpio_num_t)2;       // status led
  const gpio_num_t serialRx = (gpio_num_t)19; // bus serial rx line - coded in ibustrx referenced lib
  const gpio_num_t serialTx = (gpio_num_t)18; // bus serial tx line - coded in ibustrx referenced lib

#define SwitchCount 0
  SwitchConfig switches[SwitchCount] = {};

#define ListenChannelsCount 7
  const char *listenChannels[ListenChannelsCount] = {
    "GN02475inv/out/inverter", //should come in every 1/3 seccond
    "GN02475acc/status",
    "GN02475acc/out/sensors/adc_vacuum", //should come in every 1/2 second
    "GN02475ivts12/out/collectors/power",
    "GN02475ivts12/out/collectors/voltage",
    "GN02475ivtsHV/out/collectors/power10",
    "GN02475ivtsHV/out/collectors/voltage"
    };

#define SensorCount 3

  SensorConfig sensors[SensorCount] = {
      {devicet::adc_voltage, "adc_voltage", 35, sensort::voltage},
      {devicet::reverse_light, "reverse_light", 36, sensort::digital},
      {devicet::adc_ntc, "adc_temp", 34, sensort::temperature}};

  const gpio_num_t digipot_sck = (gpio_num_t)21;
  const gpio_num_t digipot_sdi = (gpio_num_t)23; // not used
  const gpio_num_t digipot_sdo = (gpio_num_t)22; //

  const gpio_num_t digipot_cs1 = (gpio_num_t)32;
  const gpio_num_t digipot_cs2 = (gpio_num_t)33;

#define CollectorCount 4
  CollectorConfig collectors[CollectorCount] = {
      {VOLTAGE_12, 500},
      {POWER_12, 100},
      {VOLTAGE_HV, 500},
      {POWER_HV, 100}};

  int getCollectorIndex(const char *name)
  {
    for (size_t i = 0; i < CollectorCount; i++)
    {
      if (strcmp(collectors[i].name, name) == 0)
        return i;
    }
    return -1;
  }

  int getSensorIndex(devicet device)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (sensors[i].device == device)
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
  const int statusPublish = 1000; // interval at which status is published to MQTT
  int click_onceDelay = 1000;     // milliseconds
};

extern Settings settings;
extern Intervals intervals;

#endif /* APPCONFIG_H_ */
