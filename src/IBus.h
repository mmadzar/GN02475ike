#ifndef IBUSBASE_H_
#define IBUSBASE_H_

#include "appconfig.h"
#include <IbusTrx.h>
#include "MonitorMessage.h"
#include "shared/Bytes2WiFi.h"

class IBus
{
private:
  MonitorMessage *monitored[5];
  Bytes2WiFi *b2w;

public:
  IBus();
  void setup(Bytes2WiFi* wifiport);
  void handle();
  IbusTrx ibusTrx;
};

#endif /* IBUSBASE_H_ */
