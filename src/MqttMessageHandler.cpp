#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message)
{
  if (command == "restart" && String(message).toInt() == 1)
    ESP.restart();
  else if (command == "reconnect" && String(message).toInt() == 1)
    WiFi.disconnect(false, false);
  else if (command == "coolant_temp")
  {
    status.coolant_temp = String(message).toInt();
  }
  else if (command == "rpm")
  {
    status.rpm = String(message).toInt();
  }
  else if (command == "litres1")
  {
    status.litres1 = String(message).toDouble();
  }
  else if (command == "litres2")
  {
    status.litres2 = String(message).toDouble();
  }
  else if (command == "ohmPerLiter1")
  {
    status.ohm_per_liter1 = String(message).toInt();
  }
  else if (command == "ohmPerLiter2")
  {
    status.ohm_per_liter2 = String(message).toInt();
  }
  else if (command == "digipot1")
  {
    status.digipot1 = String(message).toInt();
  }
  else if (command == "digipot2")
  {
    status.digipot2 = String(message).toInt();
  }
  else if (command == "ibus")
  {
    int length = String(message).length();
    Serial.println(length);
    char message[length + 1];
    for (size_t cnt = 0; cnt < length + 1; cnt++)
    {
      // convert byte to its ascii representation
      sprintf(&message[cnt], "%C", message[cnt]);
    }

    // convert hex representation of message to bytes
    // Ex. 41 41 41 41 41 -> AAAAA
    Serial.println(String(message));
    char tc[2] = {0x00, 0x00};
    for (size_t i = 0; i < (length) / 3; i++)
    {
      tc[0] = char(message[i * 3]);
      tc[1] = char(message[(i * 3) + 1]);
      status.ibusSend[i] = strtol(tc, 0, 16);
    }
    status.receivedCount++;
  }
}
