#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "coolant_temp") == 0)
  {
    status.coolant_temp = String(message).toInt();
  }
  else if (strcmp(command, "litres1") == 0)
  {
    status.litres1 = String(message).toDouble();
  }
  else if (strcmp(command, "litres2") == 0)
  {
    status.litres2 = String(message).toDouble();
  }
  else if (strcmp(command, "ohmPerLiter1") == 0)
  {
    status.ohm_per_liter1 = String(message).toInt();
  }
  else if (strcmp(command, "ohmPerLiter2") == 0)
  {
    status.ohm_per_liter2 = String(message).toInt();
  }
  else if (strcmp(command, "digipot1") == 0)
  {
    status.digipot1 = String(message).toInt();
  }
  else if (strcmp(command, "digipot2") == 0)
  {
    status.digipot2 = String(message).toInt();
  }
  else if (strcmp(command, "ibus") == 0)
  {
    char messagec[length + 1];
    for (size_t cnt = 0; cnt < length + 1; cnt++)
    {
      // convert byte to its ascii representation
      sprintf(&messagec[cnt], "%C", message[cnt]);
    }

    // convert hex representation of message to bytes
    // Ex. 41 41 41 41 41 -> AAAAA
    char tc[2] = {0x00, 0x00};
    for (size_t i = 0; i < (length) / 3; i++)
    {
      tc[0] = char(message[i * 3]);
      tc[1] = char(message[(i * 3) + 1]);
      status.ibusSend[i] = strtol(tc, 0, 16);
    }
  }
}
