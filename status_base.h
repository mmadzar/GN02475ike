#ifndef STATUS_BASE_H_
#define STATUS_BASE_H_

#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

class StatusBase
{
public:
    struct tm timeinfo;

    char upsince[29];
    char connectedsince[29];
    long connectCount = 0;
    long loops = 0;
    long missedSend = 0;
    long currentMillis = 0;
    uint32_t freeMem = 0;
    uint32_t minFreeMem = 0;
    const char *ipAddress = "255.255.255.255";
    const char *gatewayAddress = "255.255.255.255";
    String SSID = "";
    int8_t rssi = 0;
    long receivedCount = 0;
    StaticJsonDocument<1024> doc;

    JsonObject PrepareRoot()
    {
        JsonObject root = doc.to<JsonObject>();
        // refresh values
        rssi = WiFi.RSSI();
        freeMem = esp_get_free_heap_size();
        minFreeMem = esp_get_minimum_free_heap_size();
        root["upSince"] = upsince;
        root["connectedSince"] = connectedsince;
        root["connectCount"] = connectCount;
        root["loops"] = loops;
        root["uptime"] = (currentMillis / 600) / 100.00;
        root["freeMem"] = freeMem;
        root["minFreeMem"] = minFreeMem;
        root["ipAddress"] = ipAddress;
        root["gateway"] = gatewayAddress;
        root["location"] = "car";
        root["SSID"] = SSID;
        root["RSSI"] = rssi;
        root["receivedCount"] = receivedCount;

        return root;
    }
};

#endif /* STATUS_BASE_H_ */
