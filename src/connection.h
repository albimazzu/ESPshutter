#ifndef CONNECTION_H
#define CONNECTION_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "WebSerial.h"

class Connection {
private:
    unsigned long lastClientCheckTime;  // Time of the last check for connected clients
    unsigned long apTimeoutDuration = 300000;  // 5 minutes in milliseconds

public:
    // Initializes the WiFi connection
    void initWiFi(const char* ssid, const char* password);

    // Deinitializes the WiFi connection
    void deinitWiFi();

    // Initializes the WiFi in Access Point (AP) mode with an optional timeout parameter
    void initWiFiAP(const char* ssid, const char* password, unsigned long timeoutMs = 300000);

    // Checks for inactivity in AP mode (no connected clients) and disconnects if necessary
    void loop();

    // Returns the current WiFi status (WIFI_OFF, WIFI_STA, WIFI_AP, or WIFI_AP_STA)
    wifi_mode_t getWiFiStatus();

    void initOTA();

    void deinitOTA();

};

#endif  // CONNECTION_H
