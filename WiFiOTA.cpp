// FCEC8C33F14FCDC64BCCE53E365E7
// {[w$FeAAkf{aov/
// ZzFyTv8/gsm0qf8+1YF9 - open-ssl rand 15 chars base64 encoded - total 20 chars
// Vem6G0L/RfeQtl5k0BLB

#include "WiFiOTA.h"

char *buff = new char[20];
char *buff2 = new char[20];
IPAddress ipA;
IPAddress ipAG;
bool connecting = false;
bool startedTimeUpdated = false;

WiFiOTA::WiFiOTA()
{
}

void WiFiOTA::setupWiFi()
{
  //esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B); // to get best signal
  SETTINGS.loadSettings();
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  Serial.println("WiFi setup... ");
  WiFi.setHostname(HOST_NAME);
  WiFi.persistent(false);
  WiFi.onEvent(WiFiEvent);
  handleWiFi();
}

void WiFiOTA::ReconnectWiFi()
{
  // WiFiMulti.run();
  if (!connecting)
  {
    connecting = true;
    WiFiConfig currentNetwork = SETTINGS.APlist[positionAP];
    Serial.printf("Connecting to %s...\n", currentNetwork.ssid);
    if (currentNetwork.username && strcmp(currentNetwork.username, "") != 0 && currentNetwork.username != 0x00) // enterprise network
    {
      esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)currentNetwork.username, strlen(currentNetwork.username));
      esp_wifi_sta_wpa2_ent_set_username((uint8_t *)currentNetwork.username, strlen(currentNetwork.username));
      esp_wifi_sta_wpa2_ent_set_password((uint8_t *)currentNetwork.password, strlen(currentNetwork.password));
      esp_wifi_sta_wpa2_ent_enable();  // set config settings to enable function
      WiFi.begin(currentNetwork.ssid); // connect to wifi
    }
    else
    {
      WiFi.begin(currentNetwork.ssid, currentNetwork.password);
    }
  }
}

void WiFiOTA::handleWiFi()
{
  // check for status here to avoid SSID check on run()
  if (!connecting && WiFi.status() != WL_CONNECTED) // request for status
  {
    ReconnectWiFi();
    positionAP++;
    if (positionAP == AP_COUNT)
      positionAP = 0;
  }
}

void WiFiOTA::setupOTA()
{
  // OTA configuration
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);
  ArduinoOTA.setHostname(HOST_NAME);
  ArduinoOTA.setPassword(wifiSettings.passwordOTA);
  ArduinoOTA
      .onStart([]()
               {
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd. Restarting..."); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
                 Serial.printf("Error[%u]: ", error);
                 if (error == OTA_AUTH_ERROR)
                   Serial.println("Auth Failed");
                 else if (error == OTA_BEGIN_ERROR)
                   Serial.println("Begin Failed");
                 else if (error == OTA_CONNECT_ERROR)
                   Serial.println("Connect Failed");
                 else if (error == OTA_RECEIVE_ERROR)
                   Serial.println("Receive Failed");
                 else if (error == OTA_END_ERROR)
                   Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

void WiFiOTA::handleOTA()
{
  if ((status.currentMillis - lastOTAmillis) > 1000)
  {
    ArduinoOTA.handle();
    lastOTAmillis = status.currentMillis;
  }
}

void WiFiOTA::WiFiEvent(WiFiEvent_t event)
{
  // Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
  case SYSTEM_EVENT_WIFI_READY:
    digitalWrite(pinsSettings.led, HIGH);
    Serial.println("WiFi interface ready");
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    Serial.println("Completed scan for access points.");
    //  WiFi.disconnect(false, false);
    break;
  case SYSTEM_EVENT_STA_START:
    Serial.println("WiFi client started");
    break;
  case SYSTEM_EVENT_STA_STOP:
    Serial.println("WiFi clients stopped");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Connected to access point.");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.printf("Disconnected from WiFi access point %s.\n", status.SSID);
    digitalWrite(pinsSettings.led, HIGH);
    status.ipAddress = "none";
    status.gatewayAddress = "255.255.255.255";
    status.SSID = "";
    connecting = false;
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    Serial.println("Authentication mode of access point has changed");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    status.connectCount++;
    connecting = false;
    ipA = WiFi.localIP();
    sprintf(buff, "%d.%d.%d.%d", ipA[0], ipA[1], ipA[2], ipA[3]);
    status.ipAddress = buff;
    ipAG = WiFi.gatewayIP();
    sprintf(buff2, "%d.%d.%d.%d", ipAG[0], ipAG[1], ipAG[2], ipAG[3]);
    status.gatewayAddress = buff2;
    Serial.printf("IP: %s  GW: %s\n", status.ipAddress, status.gatewayAddress);
    status.SSID = WiFi.SSID();
    digitalWrite(pinsSettings.led, LOW);
    Serial.printf("SSID: %s  RSSI: %d\n", status.SSID, WiFi.RSSI());

    // Set time
    configTime(3 * 3600, 0, "pool.ntp.org");
    getLocalTime(&(status.timeinfo));
    if (!startedTimeUpdated)
    {
      startedTimeUpdated = true;
      strftime(status.upsince, sizeof(status.upsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
    }
    strftime(status.connectedsince, sizeof(status.connectedsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println("Lost IP address and IP address is reset to 0");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
    break;
  case SYSTEM_EVENT_AP_START:
    Serial.println("WiFi access point started");
    break;
  case SYSTEM_EVENT_AP_STOP:
    Serial.println("WiFi access point  stopped");
    break;
  case SYSTEM_EVENT_AP_STACONNECTED:
    Serial.println("Client connected");
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    Serial.println("Client disconnected");
    break;
  case SYSTEM_EVENT_AP_STAIPASSIGNED:
    Serial.println("Assigned IP address to client");
    break;
  case SYSTEM_EVENT_AP_PROBEREQRECVED:
    Serial.println("Received probe request");
    break;
  case SYSTEM_EVENT_GOT_IP6:
    Serial.println("IPv6 is preferred");
    break;
  case SYSTEM_EVENT_ETH_START:
    Serial.println("Ethernet started");
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println("Ethernet stopped");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println("Ethernet connected");
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println("Ethernet disconnected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.println("Obtained IP address");
    break;
  default:
    break;
  }
}
