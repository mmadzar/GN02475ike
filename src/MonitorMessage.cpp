#include "MonitorMessage.h"

MonitorMessage::MonitorMessage()
{
}

bool MonitorMessage::equals(byte *msg)
{
  for (size_t i = 0; i < monitorMessageSize; i++)
  {
    if (i != 1 && pattern[i] != msg[i]) // skip length byte
    {
      return false;
    }
  }
  return true;
}

uint8_t MonitorMessage::getMaxSize()
{
  if (type == IKE)
    return 16;
  else if (type == MIDOBC)
    return 20;
  else
    return 8;
}

Message MonitorMessage::getReplacementMessage()
{
  uint8_t sizeForBUS = getMaxSize() + 3 + 2; // 2 is for destination and checksum and 3 is for additional descriptive bytes
  Message(msg);
  uint8_t byteCounter = 0;

  for (size_t i = 0; i < monitorMessageSize; i++)
  {
    if (i != 1) // place for length byte
    {
      msg.message[byteCounter++] = pattern[i];
    }
    else // calculate length
    {
      msg.message[byteCounter++] = sizeForBUS + (type == IKE ? 1 : 0); // 1 is for extra IKE byte (gong)
    }
  }

  if (type == IKE) // add IKE extra byte
    msg.message[byteCounter++] = 0x00;

  uint8_t startCounter = byteCounter;
  for (size_t i = 0; i < getMaxSize(); i++)
  {
    // find message size
    String ms = String(message);
    const uint8_t ml = ms.length() - 2;
    if (byteCounter - startCounter > ml)
    {
      // fill empty byte - space
      msg.message[byteCounter++] = 0x20;
    }
    else
    {
      msg.message[byteCounter++] = message[i];
    }
    msg.message[byteCounter++] = 0x00; // place for checksum
  }
  msg.length = byteCounter;
  return msg;
}