#include "MonitorMessage.h"

MonitorMessage::MonitorMessage(int size)
{
  monitorMessageSize = size;
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
  if (type == IKEOBC)
    return 10;
  else if (type == IKE)
    return 20;
  else if (type == MIDOBC)
    return 19;
  else
    return 9;
}

Message MonitorMessage::getReplacementMessage()
{
  uint8_t sizeForBUS = getMaxSize() + 3 + 2; // 2 is for destination and checksum and 3 is for additional descriptive bytes
  Message(msg);
  uint8_t byteCounter = 0;

  if (type == MIDOBC)
  {
    // if (strcmp(key, "verb1") == 0)
    // {
    //   msg.message[0] = 0x80;
    //   msg.message[1] = 0x16;
    //   msg.message[2] = 0xe7;
    //   msg.message[3] = 0x24;
    //   msg.message[4] = 0x00;
    //   msg.message[5] = 0x61;
    //   msg.message[6] = 0x62;
    //   msg.message[7] = 0x63;
    //   msg.message[8] = 0x64;
    //   msg.message[9] = 0x65;
    //   msg.message[10] = 0x66;
    //   msg.message[11] = 0x67;
    //   msg.message[12] = 0x68;
    //   msg.message[13] = 0x69;
    //   msg.message[14] = 0x70;
    //   msg.message[15] = 0x71;
    //   msg.message[16] = 0x72;
    //   msg.message[17] = 0x73;
    //   msg.message[18] = 0x74;
    //   msg.message[19] = 0x75;
    //   msg.message[20] = 0x76;
    //   msg.message[21] = 0x77;
    //   msg.message[22] = 0x78;
    // }
    // else if (strcmp(key, "reichw") == 0)
    // {
    //   msg.message[0] = 0x80;
    //   msg.message[1] = 0x16;
    //   msg.message[2] = 0xe7;
    //   msg.message[3] = 0x24;
    //   msg.message[4] = 0x00;
    //   msg.message[5] = 0x41;
    //   msg.message[6] = 0x42;
    //   msg.message[7] = 0x43;
    //   msg.message[8] = 0x44;
    //   msg.message[9] = 0x45;
    //   msg.message[10] = 0x46;
    //   msg.message[11] = 0x47;
    //   msg.message[12] = 0x48;
    //   msg.message[13] = 0x49;
    //   msg.message[14] = 0x50;
    //   msg.message[15] = 0x51;
    //   msg.message[16] = 0x52;
    //   msg.message[17] = 0x53;
    //   msg.message[18] = 0x54;
    //   msg.message[19] = 0x55;
    //   msg.message[20] = 0x56;
    //   msg.message[21] = 0x57;
    //   msg.message[22] = 0x58;
    // }
    // msg.length = 23;
  }
  /*
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
    */
  return msg;
}