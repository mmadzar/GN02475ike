#include "IBus.h"
long lastRemoteUnlockPress = 0;
int monitorMessagesCount = 5;

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

uint8_t IKEdisplay[27] = {
    0x30, 0x19, 0x80, 0x1A, 0x35, 0x00,
    0x20, // first character in message
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x00 // checksum placeholder
};

// 3F 06 00 0C 00 3E 01 [0A]
uint8_t TrunkWindowUnlock[7] = {
    M_DIA, 0x06, M_GM5, 0x0C, 0x00, 0x3E, 0x01};

uint8_t ReverseLightsOn[17] = {
    0x3f, 0x0f, 0xd0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x17, 0x34, 0x00, 0x00};
uint8_t ReverseLightsOff[17] = {
    0x3f, 0x0f, 0xd0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x34, 0x00, 0x00};

MonitorMessage ikeVERB1(5);
MonitorMessage ikeVERB2(5);
MonitorMessage ikeREICHW(5);
MonitorMessage midVERB1(5);
MonitorMessage midVERB2(5);
MonitorMessage midREICHW(5);
uint8_t tempBusMsg[50];

IBus::IBus()
{
}

void IBus::setup(Bytes2WiFi &wifiport, Bytes2WiFi &portBytes, Bytes2WiFi &portDebug)
{
  b2w = &wifiport;
  b2wbinary = &portBytes;
  b2wdebug = &portDebug;
  // = {0x80, 0x00, 0xe7, 0x24, 0x04};
  midVERB1.pattern[0] = 0x80;
  midVERB1.pattern[1] = 0x00;
  midVERB1.pattern[2] = 0xe7;
  midVERB1.pattern[3] = 0x24;
  midVERB1.pattern[4] = 0x04;
  midVERB1.type = MIDOBC;
  midVERB1.key = "verb1";

  //{0x80, 0x00, 0xe7, 0x24, 0x05};
  midVERB2.pattern[0] = 0x80;
  midVERB2.pattern[1] = 0x00;
  midVERB2.pattern[2] = 0xe7;
  midVERB2.pattern[3] = 0x24;
  midVERB2.pattern[4] = 0x05;
  midVERB2.type = MIDOBC;
  midVERB2.key = "verb2";

  //{0x80, 0x00, 0xe7, 0x24, 0x06};
  midREICHW.pattern[0] = 0x80;
  midREICHW.pattern[1] = 0x00;
  midREICHW.pattern[2] = 0xe7;
  midREICHW.pattern[3] = 0x24;
  midREICHW.pattern[4] = 0x06;
  midREICHW.type = MIDOBC;
  midREICHW.key = "reichw";

  //{0x80, 0x00, 0xc0, 0x23, 0x04};
  ikeVERB1.pattern[0] = 0x80;
  ikeVERB1.pattern[1] = 0x00;
  ikeVERB1.pattern[2] = 0xc0;
  ikeVERB1.pattern[3] = 0x23;
  ikeVERB1.pattern[4] = 0x04;
  ikeVERB1.type = IKEOBC;
  ikeVERB1.key = "verb1";

  // {0x80, 0x00, 0xc0, 0x23, 0x05};
  ikeVERB2.pattern[0] = 0x80;
  ikeVERB2.pattern[1] = 0x00;
  ikeVERB2.pattern[2] = 0xc0;
  ikeVERB2.pattern[3] = 0x23;
  ikeVERB2.pattern[4] = 0x05;
  ikeVERB2.type = IKEOBC;
  ikeVERB2.key = "verb2";

  //{0x80, 0x00, 0xc0, 0x23, 0x06};
  ikeREICHW.pattern[0] = 0x80;
  ikeREICHW.pattern[1] = 0x00;
  ikeREICHW.pattern[2] = 0xc0;
  ikeREICHW.pattern[3] = 0x23;
  ikeREICHW.pattern[4] = 0x06;
  ikeREICHW.type = IKEOBC;
  ikeREICHW.key = "reichw";

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

    uint8_t msize = m.length() - 1; // length byte includes destination

    // check ignition status and pwr ON/OFF
    if (m.source() == M_IKE && m.destination() == M_ALL && m.length() == 4 && m.b(0) == 0x11)
    {
      if (m.b(1) == 0x00)
      {
        // key off KL-30 - don't get that on GN02475
      }
      else if (m.b(1) == 0x01)
      {
        // key pos1 KL-R
        // power OFF inverter using acc.msft1
        status.inverterPWR = false; // msg sent in mqtt msg handler
      }
      else if (m.b(1) == 0x03)
      {
        // key pos2 KL-15
        // power ON inverter using acc.msft1
        status.inverterPWR = true; // msg sent in mqtt msg handler
      }
      else if (m.b(1) == 0x07)
      {
        // key pos3 - start - KL-50
        // handled using signal from ignition directly on inverter start line
      }
    }

    // if (msize > 5) // TODO include message length from message settings
    // {
    //   tempBusMsg[0] = m.source();
    //   tempBusMsg[1] = m.length();
    //   tempBusMsg[2] = m.destination();
    //   // TODO include message length from message settings
    //   tempBusMsg[3] = m.b(0);
    //   tempBusMsg[4] = m.b(1);
    //   for (size_t i = 0; i < monitorMessagesCount; i++)
    //   {
    //     if (monitored[i]->equals(tempBusMsg))
    //     {
    //       monitored[i]->received = true;
    //       monitored[i]->replaced = false;
    //       monitored[i]->timestamp = status.currentMillis;
    //       // detected message
    //       status.receivedCount++;
    //     }
    //   // }
    // }

    uint32_t now = micros();
    b2wbinary->addBuffer(m.source());
    b2wbinary->addBuffer(m.length());
    b2wbinary->addBuffer(m.destination());
    for (size_t i = 0; i < m.length() - 1; i++)
      b2wbinary->addBuffer(m.b(i));
    b2wbinary->addBuffer("\r\n", 2); // add new line for serial monitor
    // b2wbinary->send();

    // format for sending to parse by SavvyCan
    uint32_t frameId = m.source();
    frameId |= 1 << 31;
    if (msize > 15)
      msize = 15;
    uint8_t mlength = msize + 1;

    b2w->addBuffer(0xf1);
    b2w->addBuffer(0x00); // 0 = canbus frame sending
    b2w->addBuffer(now & 0xFF);
    b2w->addBuffer(now >> 8);
    b2w->addBuffer(now >> 16);
    b2w->addBuffer(now >> 24);
    b2w->addBuffer(frameId & 0xFF);
    b2w->addBuffer(frameId >> 8);
    b2w->addBuffer(frameId >> 16);
    b2w->addBuffer(frameId >> 24);
    b2w->addBuffer((mlength - 1) + (uint8_t)(((int)2) << 4)); // 2 ibus address
    b2w->addBuffer(m.destination());
    for (int c = 0; c < (m.length() - 1) - 1; c++) // we don't need checksum and want complete message in serial
      b2w->addBuffer(m.b(c));
    b2w->addBuffer(0x0a); // in place of checksum - new line in serial monitor

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
  /*
    for (size_t i = 0; i < monitorMessagesCount; i++)
    {
      if (monitored[i]->received && !monitored[i]->replaced && status.currentMillis - monitored[i]->timestamp > 1000) // wait 500 ms before overwriting original message to avoid bus errors
      {
        // if (monitored[i]->type == MIDOBC)
        // {
        b2wdebug->addBuffer("received...\n", 12);
        b2wdebug->send();
        // Message m = monitored[i]->getReplacementMessage();
        // b2wdebug->addBuffer((String(m.length) + "\r\n").c_str(), String(m.length).length() + 3);
        // if (m.length > 0)
        // {
        //   char dr[128];
        //   b2wdebug->addBuffer("responding len: ", 16);
        //   b2wdebug->addBuffer(m.length);
        //   b2wdebug->addBuffer(".\r\n", 3);
        //   ibusTrx.write(m.message);
        //   b2wdebug->addBuffer("responded.\n", 11);
        // }
        Message(m);
        status.counts = 100;
        b2wdebug->addBuffer("\n1\n", 3);
        b2wdebug->send();
        b2wdebug->addBuffer(monitored[i]->key, 5);
        b2wdebug->addBuffer("\n2\n", 3);
        b2wdebug->send();
        b2wdebug->addBuffer(monitored[i]->key, String(monitored[i]->key).length());
        b2wdebug->addBuffer("\n3\n", 3);
        b2wdebug->send();
        if (strcmp(monitored[i]->key, "verb1") == 0)
        {
          b2wdebug->addBuffer("2", 1);
          b2wdebug->send();
          m.message[0] = 0x80;
          m.message[1] = 0x15;
          m.message[2] = 0xe7;
          m.message[3] = 0x24;
          m.message[4] = 0x06;
          m.message[5] = 0x00;
          m.message[6] = 0x62;
          m.message[7] = 0x63;
          m.message[8] = 0x64;
          m.message[9] = 0x65;
          m.message[10] = 0x66;
          m.message[11] = 0x67;
          m.message[12] = 0x68;
          m.message[13] = 0x69;
          m.message[14] = 0x70;
          m.message[15] = 0x71;
          m.message[16] = 0x72;
          m.message[17] = 0x73;
          m.message[18] = 0x74;
          m.message[19] = 0x75;
          m.message[20] = 0x76;
          m.message[21] = 0x77;
          m.message[22] = 0x78;
          status.counts = 200;
          b2wdebug->addBuffer("3", 1);
          b2wdebug->send();
        }
        else if (strcmp(monitored[i]->key, "reichw") == 0)
        {
          b2wdebug->addBuffer("4", 1);
          b2wdebug->send();
          m.message[0] = 0x80;
          m.message[1] = 0x15;
          m.message[2] = 0xe7;
          m.message[3] = 0x24;
          m.message[4] = 0x06;
          m.message[5] = 0x00;
          m.message[6] = 0x42;
          m.message[7] = 0x43;
          m.message[8] = 0x44;
          m.message[9] = 0x45;
          m.message[10] = 0x46;
          m.message[11] = 0x47;
          m.message[12] = 0x48;
          m.message[13] = 0x49;
          m.message[14] = 0x50;
          m.message[15] = 0x51;
          m.message[16] = 0x52;
          m.message[17] = 0x53;
          m.message[18] = 0x54;
          m.message[19] = 0x55;
          m.message[20] = 0x56;
          m.message[21] = 0x57;
          m.message[22] = 0x58;
          status.counts = 300;
          b2wdebug->addBuffer("5", 1);
          b2wdebug->send();
        }
        m.length = 22;
        ibusTrx.write(m.message);
        b2wdebug->addBuffer("responded.\n", 11);
        b2wdebug->send();
        // reset monitor
        monitored[i]->received = false;
        monitored[i]->replaced = true;
        break; // replace just one message in cycle
      }
    }

  */
  // update status every half a second if online
  if (strcmp(status.SSID, "") != 0 && status.currentMillis - lastIkeDisplayUpdate > 500)
  {
    lastIkeDisplayUpdate = status.currentMillis;
    bool displayChanged = false;
    for (size_t i = 0; i < 21; i++)
    {
      if (IKEdisplay[i + 6] != (uint8_t)status.ikeDisplay[i])
      {
        IKEdisplay[i + 6] = (uint8_t)status.ikeDisplay[i];
        displayChanged = true;
      }
    }

    // !!!!!!!!!!!!! increased tx buffer size in ibustrx to from 16 to 64 to accept longer messages
    if (displayChanged)
    {
      ibusTrx.write(IKEdisplay);
    }
  }

  if (status.ibusSend[1] != 0x00)
  {
    ibusTrx.write(status.ibusSend);
    status.ibusSend[1] = 0x00;
  }

  // check for reverse gear select and turn on reverse lights on/off
  if (status.reverseLights != lastReverseLights)
  {
    lastReverseLights = status.reverseLights;
    if (lastReverseLights)
    {
      ibusTrx.write(ReverseLightsOn);
      reverseLightsLastSent = status.currentMillis;
    }
    else
      ibusTrx.write(ReverseLightsOff);
    status.receivedCount++;
  }
  else if (status.reverseLights && status.currentMillis - reverseLightsLastSent > 10000)
  {
    // sending ON command eveery 10 seconds as DIA command has effect around 15s
    ibusTrx.write(ReverseLightsOn);
    reverseLightsLastSent = status.currentMillis;
    status.counts++;
  }
}
