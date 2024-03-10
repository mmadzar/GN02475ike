#ifndef IBUSBASE_H_
#define IBUSBASE_H_

#include "appconfig.h"
#include "shared/base/Collector.h"
#include <IbusTrx.h>
#include "MonitorMessage.h"
#include "shared/Bytes2WiFi.h"
#include "shared/MqttPubSub.h"

class IBus
{
private:
  Settings ikeSettings;
  Bytes2WiFi *b2w;
  Bytes2WiFi *b2wbinary;
  long lastIkeDisplayUpdate = 0;
  bool lastReverseLights = false;
  long reverseLightsLastSent = 0;
  long previousMillis = 0;     // last time a IBus Message was send
  uint64_t lastSentIBusLog = 0; // last time when logged IBus messages are sent over WiFi
  
public:
  IBus();
  void setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport, Bytes2WiFi &portBytes);
  void handle();
  MonitorMessage *monitored[5];
  IbusTrx ibusTrx;
};

#endif /* IBUSBASE_H_ */
