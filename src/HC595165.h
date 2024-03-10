#ifndef HC595265_H_
#define HC595265_H_

#include <Arduino.h>
#include "shared/base/Switch.h"
#include "shared/base/Sensor.h"
#include "shared/base/Collector.h"
#include "shared/configtypes/configtypes.h"
#include "appconfig.h"
#include <ArduinoJson.h>
#include "status.h"
#include "shared/MqttPubSub.h"
#include "shared/Bytes2WiFi.h"

class HC595165
{
private:
    Switch *relays[SwitchCount];
    SwitchConfig *relayConfigs[SwitchCount];
    Sensor *inputs[SensorCount];
    SensorConfig *inputConfigs[SensorCount];
    u8_t relaysValue = 0;
    u8_t sensorsValue = 255;
    long lastRead = 0;

public:
    HC595165();
    void handle();
    void setup(class MqttPubSub &mqtt_client);
};

#endif /* HC595165_H_ */