#include "CanBus.h"

StaticJsonDocument<512> docJ;
char tempBufferCan[512];
MqttPubSub *mqttClientCan;
Settings settingsCollectors;

CanBus::CanBus()
{
  init();
}

void CanBus::init()
{
  CAN0.setCANPins(settings.can0_rx, settings.can0_tx);
}

void CanBus::setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport)
{
  mqttClientCan = &mqtt_client;
  b2w = &wifiport;

  CAN0.begin(500000);
  CAN0.watchFor(); // watch all
}

void CanBus::handle()
{
  CAN_FRAME frame;
  if (CAN0.read(frame))
  {
    // store message to buffer
    b2w->addBuffer(0xf1);
    b2w->addBuffer(0x00); // 0 = canbus frame sending
    uint64_t now = status.getTimestampMicro();
    b2w->addBuffer(now & 0xFF);
    b2w->addBuffer(now >> 8);
    b2w->addBuffer(now >> 16);
    b2w->addBuffer(now >> 24);
    b2w->addBuffer(frame.id & 0xFF);
    b2w->addBuffer(frame.id >> 8);
    b2w->addBuffer(frame.id >> 16);
    b2w->addBuffer(frame.id >> 24);
    b2w->addBuffer(frame.length + (uint8_t)(((int)1) << 4)); // 2 ibus address
    for (int c = 0; c < frame.length; c++)
      b2w->addBuffer(frame.data.uint8[c]);
    b2w->addBuffer(0x0a); // new line in serial monitor
  }
  if (b2w->position > 5 && (status.currentMillis - lastSentCanLog >= intervals.Can2Mqtt || b2w->position >= 1900))
  {
    lastSentCanLog = status.currentMillis;
    if (status.can2mqtt_enabled)
      mqttClientCan->sendBytesToTopic(String(wifiSettings.hostname) + "/out/can/raw", b2w->getContent(), b2w->position);
    b2w->send();
    b2w->position = 0;
  }
}