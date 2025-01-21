#ifndef JSONSPIFFS_TPP
#define JSONSPIFFS_TPP
#include "jsonspiffs.h"

template<typename T>
T JsonSpiffs::get(const char* key) {
    return config[key].as<T>();
}

template<typename T>
T JsonSpiffs::get(const char* key, T defaultValue) {
    if (!config[key].is<T>()) {
        config[key] = defaultValue; // Aggiungi la chiave con il valore di default
    }
    return config[key].as<T>();
}

template<typename T>
void JsonSpiffs::set(const char* key, T value) {
    config[key] = value;
}

template<typename T>
T JsonSpiffs::getNested(const char* objectKey, const char* nestedKey) {
    return config[objectKey][nestedKey].as<T>();
}

template<typename T>
T JsonSpiffs::getNested(const char* objectKey, const char* nestedKey, T defaultValue) {
    // Verifica se l'oggetto principale esiste
    if (!config.containsKey(objectKey)) {
        config[objectKey] = JsonObject(); // Crea un oggetto vuoto
    }

    // Verifica se la sottochiave esiste nell'oggetto principale
    JsonObject nestedObject = config[objectKey].as<JsonObject>();
    if (!nestedObject.containsKey(nestedKey)) {
        nestedObject[nestedKey] = defaultValue; // Aggiungi la sottochiave con il valore di default
    }

    return nestedObject[nestedKey].as<T>();
}

template<typename T>
void JsonSpiffs::setNested(const char* objectKey, const char* nestedKey, T value) {
    config[objectKey][nestedKey] = value;
}

#endif // JSONSPIFFS_TPP
