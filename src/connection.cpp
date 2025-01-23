#include "connection.h"

AsyncWebServer server(80);

// Initializes the WiFi connection
void Connection::initWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    // Wait for the connection to establish
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Deinitializes the WiFi connection
void Connection::deinitWiFi() {
    WiFi.disconnect();
    WiFi.eraseAP();
    WiFi.mode(WIFI_MODE_NULL);
}

// Initializes the WiFi in Access Point (AP) mode with an optional timeout parameter
void Connection::initWiFiAP(const char* ssid, const char* password, unsigned long timeoutMs) {
    // Set the custom or default timeout value
    apTimeoutDuration = timeoutMs;

    // Set WiFi transmission power to minimum
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);  // Set minimum transmission power

    // Get MAC address
    String mac = WiFi.macAddress();
    mac.replace(":","");
    mac = mac.substring(6);

    // Format the SSID with the last 3 bytes of the MAC address
    char fullSSID[32];  // Assuming a maximum SSID length of 31 characters + null terminator
    // snprintf(fullSSID, sizeof(fullSSID), "%s_%s", ssid, mac);
    snprintf(fullSSID, sizeof(fullSSID), "%s", ssid); //ssid only

    // Start the Access Point with the formatted SSID
    bool success = WiFi.softAP(fullSSID, password);
    
    if (success) {
        Serial.println("Access Point started successfully.");
        Serial.print("AP SSID: ");
        Serial.println(fullSSID);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
        // Set the last check time to the current time
        lastClientCheckTime = millis();
        initOTA();

    } else {
        Serial.println("Failed to start Access Point.");
    }
}

// Checks for inactivity in AP mode (no connected clients) and disconnects if no clients for 5 minutes
void Connection::loop() {
    unsigned long currentTime = millis();
    
    // Get the number of connected clients
    int numClients = WiFi.softAPgetStationNum();

    // Check if no clients have been connected for more than the timeout period
    if (numClients == 0 && (currentTime - lastClientCheckTime > apTimeoutDuration) && WiFi.getMode() == WIFI_AP) {
        Serial.println("No clients connected for " + String(apTimeoutDuration/1000) + " seconds. Deinit wifi module");
        deinitWiFi();
    } else if (numClients > 0) {
        // Update the last client check time if there are clients connected
        lastClientCheckTime = currentTime;
    }    
    ElegantOTA.loop();
}

// Returns the current WiFi status (WIFI_OFF, WIFI_STA, WIFI_AP, or WIFI_AP_STA)
wifi_mode_t Connection::getWiFiStatus() {
    // Return the current WiFi mode
    return WiFi.getMode();
}

void Connection::initOTA(){
    WebSerial.begin(&server);
    ElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
}