#ifndef VOLTAGESENSOR_H_
#define VOLTAGESENSOR_H_

#include <Arduino.h>

#include "appconfig.h"
#include "status.h"
#include "shared/configtypes/configtypes.h"
#include "shared/base/Sensor.h"
#include "shared/MqttPubSub.h"

class Sensors
{
private:
  Sensor *devices[SensorCount];
  SensorConfig *configs[SensorCount];
  Settings settings;
  double read(Sensor *sensor);
  double calculateTemperature(SensorConfig *sc, int adc_value);
  double calculateVoltage(SensorConfig *sc, int adc_value);
  long tempValues[SensorCount];

  double calcValue = 0;

public:
  Sensors();
  void setup(class MqttPubSub &mqtt_client);
  void handle();
};

#endif /* VOLTAGESENSOR_H_ */
