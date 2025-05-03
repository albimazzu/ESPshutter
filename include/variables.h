#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

#define DEFAULT_T_fullShutterMove 10000     //default time of a full shutter move
#define DEFAULT_T_openingShutterMove 5000   //default time of shutter opening move
#define DEFAULT_nModbusId   1               //default modbus id
#define DEFAULT_AP_SSID  "ESP_shutter"
#define DEFAULT_AP_PASSWORD  "12345678"

// Global variables declaration
extern long T_fullShutterMove;
extern long T_openingShutterMove;
extern int nModbusId;
extern String wifiApSSID;
extern String wifiApPassword;

#endif // VARIABLES_H