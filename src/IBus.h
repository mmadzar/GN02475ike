#ifndef IBUSBASE_H_
#define IBUSBASE_H_

#include "appconfig.h"
#include <IbusTrx.h>
#include "MonitorMessage.h"
#include "shared/Bytes2WiFi.h"

class IBus
{
private:
  Bytes2WiFi *b2w;
  Bytes2WiFi *b2wbinary;
  Bytes2WiFi *b2wdebug;
  long lastIkeDisplayUpdate = 0;

public:
  IBus();
  void setup(Bytes2WiFi &wifiport, Bytes2WiFi &portBytes, Bytes2WiFi &portDebug);
  void handle();
  MonitorMessage *monitored[5];
  IbusTrx ibusTrx;
};

#endif /* IBUSBASE_H_ */
