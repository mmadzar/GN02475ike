#ifndef MQTTMESSAGEHANDLER_H_
#define MQTTMESSAGEHANDLER_H_

#include "Arduino.h"
#include "status.h"

typedef struct
{
    int size = 5;
    char value[32]; // reserve max space
    char defaultValue[32];
    long lastUpdated;

    void init(int maxSize, const char *defaultValue)
    {
        size = maxSize;
        sprintf(this->defaultValue, defaultValue);
        sprintf(value, defaultValue);
        // String(defaultValue).toCharArray(value, size);
        lastUpdated = status.currentMillis;
    }

    void setValue(const char *newvalue)
    {
        sprintf(value, newvalue);
        // String(newvalue).toCharArray(value, size);
        lastUpdated = status.currentMillis;
    };

    void handle()
    {
        if (status.currentMillis - lastUpdated > 2000) // fade status after two second
        {
            char buff[size + 1] = "                ";
            for (size_t i = 0; i < size; i++)
                buff[i] = '!';

            buff[size] = 0x00;
            setValue(String(buff).c_str());
        }
    }
} DisplayPartInfo;

class MqttMessageHandler
{
private:
    static int coolantPump;
    void updateDisplay();

public:
    MqttMessageHandler();
    void handle();
    static void HandleMessage(const char *topic, const char *message, int length);
    static void callback(char *topic, byte *message, unsigned int length);
};

#endif /* MQTTMESSAGEHANDLER_H_ */
