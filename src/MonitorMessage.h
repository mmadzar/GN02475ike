#ifndef MONITORMESSAGE_H_
#define MONITORMESSAGE_H_

#include "appconfig.h"
#include "status.h"
#include "DisplayType.h"

#define monitorMessagesCount 5 // should be 6 but crashes when set message for last item in collection
#define monitorMessageSize 5

class Message
{
public:
  uint8_t message[50] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 0;
};

class MonitorMessage
{
private:
public:
  uint8_t pattern[monitorMessageSize] = {0x00, 0x00, 0x00, 0x00, 0x00};
  char *key;
  char *message = "                   ";
  bool received = false;
  bool replaced = false;
  long timestamp = 0;
  uint8_t type = IKE;

  MonitorMessage();
  bool equals(byte *msg);
  Message getReplacementMessage();
  uint8_t getMaxSize();
};

#endif
