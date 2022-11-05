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
  MonitorMessage *monitored[5];

public:
  IBus();
  void setup(Bytes2WiFi &wifiport);
  void handle();
  IbusTrx ibusTrx;
};

#endif /* IBUSBASE_H_ */
