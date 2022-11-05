#ifndef MQTTMESSAGEHANDLER_H_
#define MQTTMESSAGEHANDLER_H_

class MqttMessageHandler
{

public:
    MqttMessageHandler();
    void static HandleMessage(const char *topic, const char *message, int length);
};

#endif /* MQTTMESSAGEHANDLER_H_ */
