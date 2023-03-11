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
static const char *pwrOff = "OFF  ";
static const char *waitingInverterMessage = ".....";
static StaticJsonDocument<1024> mhdoc;
char buffMMH[20];

Collector *collectors[CollectorCount];
CollectorConfig *configs[CollectorCount];

Settings settingsCollectors;

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
int ivtsPower = 0;
int ivtsVoltage = 0;

void init()
{
  for (size_t i = 0; i < CollectorCount; i++)
  {
    CollectorConfig *sc = &settings.collectors[i];
    configs[i] = new CollectorConfig(sc->name, sc->sendRate);
    collectors[i] = new Collector(*configs[i]);
    collectors[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                            { status.collectors[settingsCollectors.getCollectorIndex(name)] = value; });
    collectors[i]->setup();
  }

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
  dpIvtsOnline.init(5, " I. ");
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
    // status.receivedCount++;
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

const char *getLastTwoChars(int value)
{
  String s(value);
  if (s.length() > 2)
    return strdup(s.substring(s.length() - 2, s.length()).c_str());
  else
    return strdup(String(value).c_str());
}

bool isBmsValid()
{
  // TODO for testing NO BMSes check
  return true;
  // return dpBms1.value[0] == '0' && dpBms2.value[0] == '0' && dpBms3.value[0] == '0' && dpBms4.value[0] == '0';
}

void getValue(const char *key, const char *message, unsigned int length)
{
  collectors[settingsCollectors.getCollectorIndex(key)]->handle(String(message).toInt(), status.getTimestampMicro());
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{
  String t = String(topic);
  if (t.endsWith("inv/out/inverter"))
  {
    dpInverter.lastUpdated = status.currentMillis;
    // parse inverter status message
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
  else if (t.endsWith("acc/status") || t.endsWith("acc/out/sensors/adc_vacuum"))
  {
    dpAccOnline.setValue("1");
    if (t.endsWith("acc/status"))
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
      if (invpwr == 0) // inverter PWR off
        dpInverter.setValue(pwrOff);
      else if (String(dpInverter.value).equals(pwrOff)) // inverter PWR ON but no message from inverter yet
        dpInverter.setValue(waitingInverterMessage);
    }
    // don't timeout inverter message if ACC live
    dpInverter.lastUpdated = status.currentMillis;
  }
  else if (t.endsWith("ivts12/out/collectors/power"))
  {
    deserializeJson(mhdoc, message);
    getValue(POWER_12, mhdoc["value"], length);
    int sharedIVTSchanged = status.collectors[settingsCollectors.getCollectorIndex(POWER_12)];
    if (abs(sharedIVTSchanged) > 10) // trigger above 10W
    {
      String s(sharedIVTSchanged < 0 ? "-" : "+");
      s.concat(abs(sharedIVTSchanged) / 10); // show .1
      if (abs(sharedIVTSchanged) < 100)
        s.concat("  ");
      else if (abs(sharedIVTSchanged) < 1000)
        s.concat(" ");
      dpIvtsOnline.setValue(s.c_str());
    }
    else
      dpIvtsOnline.setValue("    ");
  }
  else if (t.endsWith("ivts12/out/collectors/voltage"))
  {
    deserializeJson(mhdoc, message);
    getValue(VOLTAGE_12, mhdoc["value"], length);
  }
  else if (t.endsWith("ivtsHV/out/collectors/power10"))
  {
    deserializeJson(mhdoc, message);
    getValue(POWER_HV, mhdoc["value"], length);
    int sharedIVTSchanged = status.collectors[settingsCollectors.getCollectorIndex(POWER_HV)];
    if (abs(sharedIVTSchanged) > 1000) // trigger only above 1kW
    {
      String s(sharedIVTSchanged < 0 ? "-" : "+");
      s.concat(abs(sharedIVTSchanged) / 1000); // show .001
      if (abs(sharedIVTSchanged) < 10000)
        s.concat("  ");
      else if (abs(sharedIVTSchanged) < 100000)
        s.concat(" ");
      dpBms1.setValue(s.substring(0, 1).c_str());
      dpBms2.setValue(s.substring(1, 2).c_str());
      dpBms3.setValue(s.substring(2, 3).c_str());
      dpBms4.setValue(s.substring(3, 4).c_str());
    }
    else
    {
      dpBms1.setValue(" ");
      dpBms2.setValue(" ");
      dpBms3.setValue(" ");
      dpBms4.setValue(" ");
    }
  }
  else if (t.endsWith("ivtsHV/out/collectors/voltage"))
  {
    deserializeJson(mhdoc, message);
    getValue(VOLTAGE_HV, mhdoc["value"], length);
  }
}

void MqttMessageHandler::updateDisplay()
{
  //                             +- HV -+
  //                             b1b2b3b4lv
  sprintf(status.ikeDisplay, "%S %S%S%S%S%S%S %S%C",
          dpInverter.value,
          dpBms1.value, dpBms2.value, dpBms3.value, dpBms4.value,
          dpIvtsOnline.value,
          dpServoPump.value,
          dpVaccPump.value,
          (status.reverseLights ? 'r' : +' '));
}

void MqttMessageHandler::handle()
{
  for (size_t i = 0; i < DISPLAYPARTS_COUNT; i++)
  {
    parts[i]->handle();
  }
  updateDisplay();
}