#include "IBus.h"
long lastRemoteUnlockPress = 0;
// PinsSettings pinsSettings;

// define the message that we want to transmit
// the message must be defined as an array of uint8_t's (unsigned 8-bit integers)
uint8_t toggleDomeLight[6] = {
    M_DIA,              // sender ID (diagnostic interface)
    0x05,               // length of the message payload (including destination ID and checksum)
    M_GM5,              // destination ID (body control module)
    GM5_SET_IO,         // the type of message (IO manipulation)
    GM5_BTN_DOME_LIGHT, // the first parameter (the IO line that we want to manipulate)
    0x01                // second parameter
                        // don't worry about the checksum, the library automatically calculates it for you
};

uint8_t IKEdisplay[11] = {
    0x30,
    0x0A,
    0x80,
    0x1A,
    0x35,
    0x00,
    0x41,
    0x41,
    0x41,
    0x41,
    0x41
    // don't worry about the checksum, the library automatically calculates it for you
};

// 3F 06 00 0C 00 3E 01 [0A]
uint8_t TrunkWindowUnlock[7] = {
    M_DIA,
    0x06,
    M_GM5,
    0x0C,
    0x00,
    0x3E,
    0x01};

MonitorMessage(ikeVERB1);
MonitorMessage(ikeVERB2);
MonitorMessage(ikeREICHW);
MonitorMessage(midVERB1);
MonitorMessage(midVERB2);
MonitorMessage(midREICHW);
uint8_t tempBusMsg[monitorMessageSize];

IBus::IBus()
{
}

void IBus::setup()
{
  // MID BC display size - 11 chars
  // IKE display size - 20 chars

  // = {0x80, 0x00, 0xe7, 0x24, 0x04};
  ikeVERB1.pattern[0] = 0x80;
  ikeVERB1.pattern[1] = 0x00;
  ikeVERB1.pattern[2] = 0xe7;
  ikeVERB1.pattern[3] = 0x24;
  ikeVERB1.pattern[4] = 0x04;
  ikeVERB1.type = IKE;
  ikeVERB1.key = "verb1";

  //{0x80, 0x00, 0xe7, 0x24, 0x05};
  ikeVERB2.pattern[0] = 0x80;
  ikeVERB2.pattern[1] = 0x00;
  ikeVERB2.pattern[2] = 0xe7;
  ikeVERB2.pattern[3] = 0x24;
  ikeVERB2.pattern[4] = 0x05;
  ikeVERB2.type = IKE;
  ikeVERB2.key = "verb2";

  //{0x80, 0x00, 0xe7, 0x24, 0x06};
  ikeREICHW.pattern[0] = 0x80;
  ikeREICHW.pattern[1] = 0x00;
  ikeREICHW.pattern[2] = 0xe7;
  ikeREICHW.pattern[3] = 0x24;
  ikeREICHW.pattern[4] = 0x06;
  ikeREICHW.type = IKE;
  ikeREICHW.key = "reichw";

  //{0x80, 0x00, 0xc0, 0x23, 0x04};
  midVERB1.pattern[0] = 0x80;
  midVERB1.pattern[1] = 0x00;
  midVERB1.pattern[2] = 0xc0;
  midVERB1.pattern[3] = 0x23;
  midVERB1.pattern[4] = 0x04;
  midVERB1.type = MIDOBC;
  midVERB1.key = ikeVERB1.key;

  // {0x80, 0x00, 0xc0, 0x23, 0x05};
  midVERB2.pattern[0] = 0x80;
  midVERB2.pattern[1] = 0x00;
  midVERB2.pattern[2] = 0xc0;
  midVERB2.pattern[3] = 0x23;
  midVERB2.pattern[4] = 0x05;
  midVERB2.type = MIDOBC;
  midVERB2.key = ikeVERB2.key;

  //{0x80, 0x00, 0xc0, 0x23, 0x06};
  midREICHW.pattern[0] = 0x80;
  midREICHW.pattern[1] = 0x00;
  midREICHW.pattern[2] = 0xc0;
  midREICHW.pattern[3] = 0x23;
  midREICHW.pattern[4] = 0x06;
  midREICHW.type = MIDOBC;
  midREICHW.key = ikeREICHW.key;

  monitored[0] = &ikeVERB1;
  monitored[1] = &ikeVERB2;
  monitored[2] = &ikeREICHW;
  monitored[3] = &midVERB1;
  monitored[4] = &midVERB2;
  monitored[5] = &midREICHW;

  ibusTrx.begin(Serial1); // begin listening for messages

  monitored[0]->message = status.ikeVerb1;
  monitored[1]->message = status.ikeVerb2;
  monitored[2]->message = status.ikeReichw;
  monitored[3]->message = status.midVerb1;
  monitored[4]->message = status.midVerb2;
  monitored[5]->message = status.midReichw;
}

void IBus::handle()
{
  if (ibusTrx.available())
  {
    IbusMessage m = ibusTrx.readMessage(); // grab incoming messages

    // prepare message for can log file
    uint8_t msize = m.length() - 1; // length byte includes destination

    if (msize > 0)
    {

      // // send to WiFi
      // if (status.busBytesSize > 1950)
      //   status.busBytesSize = 0;
      // status.busBytes[status.busBytesSize++] = m.source();
      // status.busBytes[status.busBytesSize++] = m.length();
      // status.busBytes[status.busBytesSize++] = m.destination();

      // for (size_t k = 0; k < msize; k++)
      // {
      //   status.busBytes[status.busBytesSize++] = m.b(k);
      // }

      if (status.ledMode != 2)
        status.ledMode = 2;

      tempBusMsg[0] = m.source();
      tempBusMsg[1] = m.length();
      tempBusMsg[2] = m.destination();
      tempBusMsg[3] = m.b(0);
      tempBusMsg[4] = m.b(1);

      for (size_t i = 0; i < monitorMessagesCount; i++)
      {
        if (monitored[i]->equals(tempBusMsg))
        {
          monitored[i]->received = true;
          monitored[i]->replaced = false;
          monitored[i]->timestamp = status.currentMillis;

          // if (status.busBytesSize > 2040)
          //   status.busBytesSize = 0;
          // status.busBytes[status.busBytesSize++] = 0xAA;
          // status.busBytes[status.busBytesSize++] = i;
          // status.busBytes[status.busBytesSize++] = 0xAA;
        }
      }

      // format for sending to parse by SavvyCan
      uint32_t frameId = m.source();
      frameId |= 1 << 31;
      uint8_t mlength = m.length();
      if ((20 + mlength + status.busBytesSize) > 2045)
      {
        status.busBytesSize = 0;
      }
      status.busBytes[status.busBytesSize++] = 0xF1;
      status.busBytes[status.busBytesSize++] = 0; // 0 = canbus frame sending
      uint32_t now = micros();
      status.busBytes[status.busBytesSize++] = (uint8_t)(now & 0xFF);
      status.busBytes[status.busBytesSize++] = (uint8_t)(now >> 8);
      status.busBytes[status.busBytesSize++] = (uint8_t)(now >> 16);
      status.busBytes[status.busBytesSize++] = (uint8_t)(now >> 24);
      status.busBytes[status.busBytesSize++] = (uint8_t)(frameId & 0xFF);
      status.busBytes[status.busBytesSize++] = (uint8_t)(frameId >> 8);
      status.busBytes[status.busBytesSize++] = (uint8_t)(frameId >> 16);
      status.busBytes[status.busBytesSize++] = (uint8_t)(frameId >> 24);
      status.busBytes[status.busBytesSize++] = mlength + (uint8_t)(((int)2) << 4); // 2 ibus address
      status.busBytes[status.busBytesSize++] = m.destination();
      for (int c = 0; c < mlength; c++)
      {
        status.busBytes[status.busBytesSize++] = (uint8_t)m.b(c);
      }
      status.busBytes[status.busBytesSize++] = (uint8_t)0;
      // sprintf((char *)&status.busBytes[status.busBytesSize], "\r\n");
      // status.busBytesSize += 2;

      // // open trunk window
      // if (m.source() == M_GM5 && m.b(0) == 0x72 && m.b(1) == 0x22)
      // {
      //   long diffTime = status.currentMillis - lastRemoteUnlockPress;
      //   if (lastRemoteUnlockPress == 0 || diffTime > 2000)
      //   {
      //     lastRemoteUnlockPress = status.currentMillis;
      //     status.counts = 0;
      //   }
      //   else if (diffTime <= 2000)
      //   {
      //     lastRemoteUnlockPress = status.currentMillis;
      //     status.counts++;
      //     if (status.counts > 1)
      //     {
      //       ibusTrx.write(TrunkWindowUnlock);
      //       lastRemoteUnlockPress = 0;
      //       status.counts = 0;
      //     }
      //   }
      // }
    }
  }

  for (size_t i = 0; i < monitorMessagesCount; i++)
  {
    if (monitored[i]->received && !monitored[i]->replaced && status.currentMillis - monitored[i]->timestamp > 500) // wait 500 ms before overwriting original message to avoid bus errors
    {
      Message m(monitored[i]->getReplacementMessage());
      if (!ibusTrx.transmitWaiting())
      {
        ibusTrx.write(m.message);

        // reset monitor
        monitored[i]->received = false;
        monitored[i]->replaced = true;
        // if (status.busBytesSize > 2045)
        //   status.busBytesSize = 0;
        // status.busBytes[status.busBytesSize++] = 0xBB;
        /*for (size_t k = 0; k < m.length; k++)
        {
          status.busBytes[status.busBytesSize++] = m.message[k];
        }*/
      }
      break; // replace just one message in cycle
    }
  }

  if (status.ibusSend[1] != 0x00)
  {
    // Serial.print("Sending ");
    // for (uint8_t p = 0; p <= status.ibusSend[1]; p++)
    // {
    //   Serial.print(" ");
    //   Serial.print(status.ibusSend[p], 16);
    // }
    // Serial.println("...");
    // if (!ibusTrx.transmitWaiting())
    // {
    //   if(status.ibusSend[0]==0x3f && status.ibusSend[1]==0x0f && status.ibusSend[2]==0xd0)
    //   { // diagnostic activate message
    //     byte diagMSg[5]={
    //         0x3f,
    //         0x03,
    //         0xd0,
    //         0x0b,
    //         0x78
    //     };
    //     ibusTrx.write(diagMSg);
    //   }
      ibusTrx.write(status.ibusSend);
      status.ibusSend[1] = 0x00;
    //}
  }
}
