#include "MqttMessageHandler.h"

#define DISPLAYPARTS_COUNT 11
DisplayPartInfo *parts[DISPLAYPARTS_COUNT];
const char *invertererrors[19] = {
    "READY", // no last error
    "OVERC",
    "THRT1",
    "THRT2",
    "CANTO",
    "EMCYS",
    "MPROT",
    "DESAT",
    "OVERV",
    "ENCDR",
    "PRECH",
    "TMPHS",
    "CURRL",
    "PWMST",
    "HICU1",
    "HICU2",
    "HIRES",
    "LORES",
    "TMPM "};
static const char *commonName = "GN02475";
static const char *pwrOff = "OFF  ";
static const char *waitingInverterMessage = ".....";
static StaticJsonDocument<1024> mhdoc;
char buffMMH[20];

DisplayPartInfo(dpInverter);
DisplayPartInfo(dpCoolantPump);
DisplayPartInfo(dpInverterPWR);
DisplayPartInfo(dpBms1);
DisplayPartInfo(dpBms2);
DisplayPartInfo(dpBms3);
DisplayPartInfo(dpBms4);
DisplayPartInfo(dpAccOnline);
DisplayPartInfo(dpIvtsOnline);
DisplayPartInfo(dpServoPump);
DisplayPartInfo(dpVaccPump);
bool initDone = false;

void init()
{
  // init default values
  int i = 0;
  dpInverter.init(5, ".....");
  parts[i++] = &dpInverter;

  dpCoolantPump.init(1, ".");
  parts[i++] = &dpCoolantPump;

  dpInverterPWR.init(1, ".");
  parts[i++] = &dpInverterPWR;

  dpBms1.init(1, ".");
  dpBms2.init(1, ".");
  dpBms3.init(1, ".");
  dpBms4.init(1, ".");
  parts[i++] = &dpBms1;
  parts[i++] = &dpBms2;
  parts[i++] = &dpBms3;
  parts[i++] = &dpBms4;

  dpAccOnline.init(1, ".");
  dpIvtsOnline.init(2, "I.");
  dpServoPump.init(2, "S.");
  dpVaccPump.init(2, "V.");
  parts[i++] = &dpAccOnline;
  parts[i++] = &dpIvtsOnline;
  parts[i++] = &dpServoPump;
  parts[i++] = &dpVaccPump;

  initDone = true;
}

MqttMessageHandler::MqttMessageHandler()
{
  init();
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "litres1") == 0)
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
    Serial.printf(" >>> %S\r\n", (char *)status.ibusSend);
  }
}

const char *getLastTwoChars(int value)
{
  String s(value);
  if (s.length() > 2)
    return strdup(s.substring(s.length() - 2, s.length()).c_str());
  else
    return strdup(String(value).c_str());
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{
  // subscribed to all channels
  String t = String(topic);
  if (t.startsWith(String(commonName) + "inv"))
  {
    dpInverter.lastUpdated = status.currentMillis;
    // parse inverter status message
    if (t.equals(String(commonName) + "inv/out/inverter"))
    {
      // inverter PWR on and online or there would be no message from inverter
      deserializeJson(mhdoc, message);
      const int opmode = mhdoc["opmode"];
      const int lasterr = mhdoc["lasterr"];
      const double tmphs = mhdoc["tmphs"];

      if (opmode == 0) // inverter stopped no error or errors
        sprintf(buffMMH, "%S", invertererrors[String(lasterr).toInt()]);
      else // inverter running
        sprintf(buffMMH, "%S %S%S", getLastTwoChars(tmphs), dpCoolantPump.value, (lasterr > 0 ? "e" : " "));
      dpInverter.setValue(buffMMH);
    }
  }
  else if (t.equals(String(commonName) + "acc/status"))
  {
    deserializeJson(mhdoc, message);
    const int servop = mhdoc["switches"]["msft_servo"];
    const int vaccp = mhdoc["switches"]["msft_vacuum"];
    const int coolp = mhdoc["switches"]["msft_coolant_pwm"];
    const int invpwr = mhdoc["switches"]["msft1"];
    dpServoPump.setValue((String("S") + servop).c_str());
    dpVaccPump.setValue((String("V") + vaccp).c_str());
    dpCoolantPump.setValue(coolp > 0 ? "1" : "0");
    dpInverterPWR.setValue(String(invpwr).c_str());
    dpAccOnline.setValue("1");
    if (invpwr == 0) // inverter PWR off
      dpInverter.setValue(pwrOff);
    else if (String(dpInverter.value).equals(pwrOff)) // inverter PWR on but not messag from inverter yet
      dpInverter.setValue(waitingInverterMessage);
    dpInverter.lastUpdated = status.currentMillis;
  }
  else if (t.startsWith(String(commonName) + "ivts12"))
  {
    dpIvtsOnline.setValue("I1");
  }
}

void MqttMessageHandler::updateDisplay()
{
  sprintf(status.ikeDisplay, "%S %S%S%S%S %S %S %S ",
          dpInverter.value,
          dpBms1.value, dpBms2.value, dpBms3.value, dpBms4.value,
          dpIvtsOnline.value,
          dpServoPump.value,
          dpVaccPump.value);
}

void MqttMessageHandler::handle()
{
  for (size_t i = 0; i < DISPLAYPARTS_COUNT; i++)
  {
    parts[i]->handle();
  }
  updateDisplay();
}