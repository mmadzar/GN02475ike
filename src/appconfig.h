#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475ike111"

#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct PinsSettings
{
  const gpio_num_t led = (gpio_num_t)2;       // status led
  const gpio_num_t serialRx = (gpio_num_t)19; // bus serial rx line - coded in ibustrx referenced lib
  const gpio_num_t serialTx = (gpio_num_t)18; // bus serial tx line - coded in ibustrx referenced lib

#define SensorCount 3

  SensorConfig sensors[SensorCount] = {
      {devicet::adc_voltage, "adc_voltage", 35, sensort::voltage},
      {devicet::reverse_light, "reverse_light", 36, sensort::adc},
      {devicet::adc_ntc, "adc_temp", 34, sensort::temperature}
  };

  const gpio_num_t digipot_sck = (gpio_num_t)21;
  const gpio_num_t digipot_sdi = (gpio_num_t)-1; // not used
  const gpio_num_t digipot_sdo = (gpio_num_t)22; //

  const gpio_num_t digipot_cs1 = (gpio_num_t)32;
  const gpio_num_t digipot_cs2 = (gpio_num_t)33;

  int getSensorIndex(devicet device)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (sensors[i].device == device)
        return i;
    }
  }
};

struct Intervals
{
  const int statusPublish = 1000; // interval at which status is published to MQTT
  // const int Can2Mqtt = 1000;      // send CAN messages to MQTT every n secconds. Accumulate messages until. Set this to 0 for forwarding all CAN messages to MQTT as they are received.
  // const int CANsend = 10;         // interval at which to send CAN Messages to car bus network (milliseconds)
};

extern PinsSettings pinsSettings;
extern Intervals intervals;

#endif /* APPCONFIG_H_ */
