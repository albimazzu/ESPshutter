#include "jsonspiffs.h"

JsonSpiffs::JsonSpiffs(const char* path)
    : configFilePath(path) {}

bool JsonSpiffs::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }
    return true;
}

bool JsonSpiffs::loadConfig() {
    if (!SPIFFS.exists(configFilePath)) {
        Serial.println("Config file does not exist, creating default.");
        return saveConfig();//createDefaultConfig();
    }

    File file = SPIFFS.open(configFilePath, "r");
    if (!file) {
        Serial.println("Failed to open config file for reading");
        return false;
    }

    DeserializationError error = deserializeJson(config, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        return false;
    }

    Serial.println("Config loaded successfully");
    return true;
}

bool JsonSpiffs::saveConfig() {
    File file = SPIFFS.open(configFilePath, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    if (serializeJson(config, file) == 0) {
        Serial.println("Failed to write to config file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Config saved successfully");
    return true;
}

void JsonSpiffs::printConfig() {
    serializeJsonPretty(config, Serial);
    Serial.println();
}
