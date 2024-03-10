#include "HC595165.h"

MqttPubSub *mqttClientHC;
Settings settings;

HC595165::HC595165()
{
}

void vpio_tx_byte(u8_t byte)
{
  s32_t i;

  for (i = 0; i < 8; i++)
  {
    digitalWrite(settings.sdi, (byte & 0x01) ? HIGH : LOW);
    digitalWrite(settings.clk, LOW);
    digitalWrite(settings.clk, HIGH);
    byte >>= 1;
  }
}

u8_t vpio_tx_rx_byte(u8_t w_byte)
{
  s32_t i;
  u8_t byte = 0;

  for (i = 0; i < 8; i++)
  {
    digitalWrite(settings.sdi, (w_byte & 0x01) ? HIGH : LOW);
    w_byte >>= 1;

    byte >>= 1;
    if (digitalRead(settings.q7))
      byte |= 0x80;

    digitalWrite(settings.clk, LOW);
    digitalWrite(settings.clk, HIGH);
  }

  return byte;
}

void vpio_refresh(u8_t *igpio, u8_t *ogpio, u8_t i_bytes, u8_t o_bytes)
{
  s32_t i;
  digitalWrite(settings.rck, LOW);

  if (i_bytes > 0)
    for (i = 0; i < o_bytes; i++)
      igpio[i] = vpio_tx_rx_byte(ogpio[i]);
  else
    for (i = 0; i < o_bytes; i++)
      vpio_tx_byte(ogpio[i]);

  digitalWrite(settings.rck, HIGH);
  digitalWrite(settings.sdi, HIGH);
}

void HC595165::setup(class MqttPubSub &mqtt_client)
{
  mqttClientHC = &mqtt_client;

  pinMode(settings.sdi, OUTPUT);
  pinMode(settings.clk, OUTPUT);
  pinMode(settings.rck, OUTPUT);
  pinMode(settings.pl, OUTPUT);
  pinMode(settings.q7, INPUT);

  digitalWrite(settings.rck, HIGH);
  digitalWrite(settings.clk, HIGH);

  // init state all low
  vpio_refresh(&sensorsValue, &relaysValue, 0, 1);

  for (size_t i = 0; i < SwitchCount; i++)
  {
    SwitchConfig *sc = &settings.switches[i];
    relayConfigs[i] = new SwitchConfig(sc->device, sc->name, sc->pin, sc->channel, sc->switchtype);
    relays[i] = new Switch(i, *relayConfigs[i]);
    relays[i]->onChange([](const char *name, devicet devicetype, int value)
                        { mqttClientHC->sendMessageToTopic(String(value), String(wifiSettings.hostname) + "/out/switches/" + name); });
    // no device setup for 74HC595 shift register
    if (sc->device != devicet::relay)
      relays[i]->setup();
  }

  for (size_t i = 0; i < SensorCount; i++)
  {
    SensorConfig *sc = &settings.sensors[i];
    inputConfigs[i] = new SensorConfig(sc->device, sc->name, sc->pin, sc->R1, sc->R2, sc->sensortype);
    inputs[i] = new Sensor(*inputConfigs[i]);
    inputs[i]->onChange([](const char *name, devicet devicetype, int value)
                        { 
                          status.sensors[settings.getSensorIndex(name)]=value;
                          mqttClientHC->sendMessageToTopic(String(value), String(wifiSettings.hostname) + "/out/sensors/" + name); });
    // no device setup for 74HC165 shift register
    if (sc->device != devicet::input)
      inputs[i]->setup();
  }
}

void HC595165::handle()
{
  // read every 5 ms
  if (lastRead + 5 < status.currentMillis)
  {
    lastRead = millis();
    relaysValue = 0xFF;
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (i < 8) // write only shift register relays
        bitWrite(relaysValue, i, status.switches[i]);
      relays[i]->handle();
    }
    vpio_refresh(&sensorsValue, &relaysValue, 1, 1);
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (i < 8) // read only shift register sensors
        status.sensors[i] = bitRead(sensorsValue, i);
      inputs[i]->handle();
    }
  }
}
