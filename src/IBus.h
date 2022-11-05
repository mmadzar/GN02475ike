#ifndef IBUSBASE_H_
#define IBUSBASE_H_

#include "appconfig.h"
#include <IbusTrx.h>
#include "MonitorMessage.h"

class IBus
{
private:
   MonitorMessage *monitored[5];

public:
  IBus();
  void setup();
  void handle();
  IbusTrx ibusTrx;
};

#endif /* IBUSBASE_H_ */
