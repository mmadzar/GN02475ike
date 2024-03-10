#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "ibus") == 0)
  {
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
  else if (strcmp(command, "ibus2mqtt_enabled") == 0)
    status.ibus2mqtt_enabled = String(message).toInt() > 0;
  else if (strcmp(command, "can2mqtt_enabled") == 0)
    status.can2mqtt_enabled = String(message).toInt() > 0;
  else if (strcmp(command, "can2mqtt") == 0)
    intervals.Can2Mqtt = String(message).toInt();
  else if (strcmp(command, "ibus2mqtt") == 0)
    intervals.Ibus2Mqtt = String(message).toInt();
  else if (strcmp(command, "reverse_lights") == 0)
    status.reverse_lights = String(message).toInt() > 0;
  else if (strcmp(command, RELAY1) == 0)
    status.switches[0] = String(message).toInt();
  else if (strcmp(command, RELAY2) == 0)
    status.switches[1] = String(message).toInt();
  else if (strcmp(command, RELAY3) == 0)
    status.switches[2] = String(message).toInt();
  else if (strcmp(command, RELAY4) == 0)
    status.switches[3] = String(message).toInt();
  else if (strcmp(command, RELAY5) == 0)
    status.switches[4] = String(message).toInt();
  else if (strcmp(command, RELAY6) == 0)
    status.switches[5] = String(message).toInt();
  else if (strcmp(command, RELAY7) == 0)
    status.switches[6] = String(message).toInt();
  else if (strcmp(command, RELAY8) == 0)
    status.switches[7] = String(message).toInt();
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{
}

void MqttMessageHandler::handle()
{
}