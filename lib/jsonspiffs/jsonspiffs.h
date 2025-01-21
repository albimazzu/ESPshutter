#ifndef JSONSPIFFS_H
#define JSONSPIFFS_H

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

class JsonSpiffs {
private:
    const char* configFilePath;

public:
    JsonDocument config;

    JsonSpiffs(const char* path = "/config.json");

    bool begin();
    bool loadConfig();
    bool saveConfig();
    void printConfig();

    template<typename T>
    T get(const char* key);

    template<typename T>
    T get(const char* key, T defaultValue);

    template<typename T>
    void set(const char* key, T value);

    template<typename T>
    T getNested(const char* objectKey, const char* nestedKey);

    template<typename T>
    T getNested(const char* objectKey, const char* nestedKey, T defaultValue);

    template<typename T>
    void setNested(const char* objectKey, const char* nestedKey, T value);
};

#include "JsonSpiffs.tpp" // Include dei template separati per evitare problemi di linking

#endif // JSONSPIFFS_H
