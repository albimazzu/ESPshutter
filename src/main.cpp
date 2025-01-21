#include <Arduino.h>
#include "SPIFFS.h"
#include <FS.h>
#include <ArduinoJson.h> // JSON parsing
#include <Preferences.h>
#include "mapping.h"
#include "debounceInterrupt.h"
#include "MillisTimer.h"
#include "secrets.h"
#include "connection.h"
#include "shutter.h"
#include "variables.h"
#include "jsonspiffs.h"
#include "mbslave.h"

#define DEF_CALIBRATION_PRESSED_TIME 5000 //msec, when up/down buttons are pressed more than this time start calibration

long T_fullShutterMove;
long T_openingShutterMove;
int nModbusId;
String wifiApSSID;
String wifiApPassword;

JsonSpiffs jsonSpiffs("/config.json");

Connection wifiConnection;

MbSlave modbusSlave(Serial1, 11, 12, PIN_485DIR);

// Istanza della classe DebounceInterrupt
DebounceInterrupt upCommand(0, PIN_ACIN_1, 60); //freq in Hz
DebounceInterrupt downCommand(1, PIN_ACIN_2, 60); //freq in Hz
bool lastUpCommand = false;
bool lastDownCommand = false;

MillisTimer TIMER_Heartbeat;
MillisTimer TIMER_CalibrationTrigger;

Shutter shutter(PIN_UP_CMD, PIN_DOWN_CMD);

void physicalInputsHandler();
void heartbeat();
void debug();

//SPIFFS functions
void setupSPIFFS();
void loadConfig();
void saveConfig();


void setup() {
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESPshutter startup");
  pinMode(PIN_UP_CMD, OUTPUT);
  pinMode(PIN_DOWN_CMD, OUTPUT);
  pinMode(PIN_STATUSLED, OUTPUT);
  pinMode(PIN_485DIR, OUTPUT);

  pinMode(PIN_ACIN_1, INPUT);
  pinMode(PIN_ACIN_2, INPUT);
  pinMode(PIN_ACIN_3, INPUT);
  pinMode(PIN_ACIN_4, INPUT);
  pinMode(PIN_SENSE, INPUT);
  pinMode(PIN_USRBTN, INPUT);

  loadConfig();

  modbusSlave.begin(nModbusId);

  TIMER_Heartbeat.begin(500);  
  TIMER_CalibrationTrigger.begin(DEF_CALIBRATION_PRESSED_TIME);

  shutter.begin(T_fullShutterMove);

  //turn on wifi on startup
  //wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);

  delay(1000);
  Serial.println("Setup completed");
  
}


void loop() {

  heartbeat();
  shutter.handler();
  modbusSlave.task();
  physicalInputsHandler();

  

  // if(!digitalRead(PIN_USRBTN) && wifiConnection.getWiFiStatus() == WIFI_OFF)
  // {
  //   wifiConnection.initWiFiAP("ESP_shutter", "12345678", 60000);
  // }
  // wifiConnection.loop();
}

void physicalInputsHandler()
{

  #pragma region upCommand
  //RISING edge detection upCommand
  if(upCommand.isPressed() == true && lastUpCommand == false)
  {
    Serial.println("upCommand rising edge");
    if(shutter.isMoving())
    {
      TIMER_CalibrationTrigger.stop();
      shutter.stop();
    }
    else
    {
      shutter.commandUp();
      TIMER_CalibrationTrigger.start(); //start calibration timer
    }
  }
  //FALLING edge detection upCommand
  if(upCommand.isPressed() == false && lastUpCommand == true)
  {
    Serial.println("upCommand falling edge");
    TIMER_CalibrationTrigger.stop(); //stop calibration timer
    shutter.stopCalibration();
    if(shutter.getMoveTime() != T_fullShutterMove)
    {
      T_fullShutterMove = shutter.getMoveTime();
      jsonSpiffs.set("T_fullShutterMove", T_fullShutterMove);
      jsonSpiffs.saveConfig();
      Serial.println("New calibration time:"+String(T_fullShutterMove));
    }
  }
  lastUpCommand = upCommand.isPressed();
  #pragma endregion

  #pragma region downCommand
  //RISING edge detection downCommand
  if(downCommand.isPressed() == true && lastDownCommand == false)
  {
    Serial.println("downCommand rising edge");
    if(shutter.isMoving())
    {
      TIMER_CalibrationTrigger.stop();
      shutter.stop();
    }
    else
    {
      shutter.commandDown();
      TIMER_CalibrationTrigger.start(); //start calibration timer
    }
  }
  //FALLING edge detection downCommand
  if(downCommand.isPressed() == false && lastDownCommand == true)
  {
    Serial.println("downCommand falling edge");
    TIMER_CalibrationTrigger.stop(); //stop calibration timer
    shutter.stopCalibration();
    if(shutter.getMoveTime() != T_fullShutterMove)
    {
      T_fullShutterMove = shutter.getMoveTime();
      jsonSpiffs.set("T_fullShutterMove", T_fullShutterMove);
      jsonSpiffs.saveConfig();
      Serial.println("New calibration time:"+String(T_fullShutterMove));
    }
  }
  lastDownCommand = downCommand.isPressed();
  #pragma endregion

  if(TIMER_CalibrationTrigger.fire())
  {
    TIMER_CalibrationTrigger.stop();
    if(shutter.isMoving())
      shutter.startCalibration();
  }
}

void heartbeat()
{
  TIMER_Heartbeat.start();
  if(TIMER_Heartbeat.fire(true))
  {
    digitalWrite(PIN_STATUSLED,!digitalRead(PIN_STATUSLED));
  }  
}

void debug()
{
  if(Serial.available()>0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if(command == "up")
    {
      shutter.commandUp();
    }
    else if(command == "down")
    {
      shutter.commandDown();
    }
    else if(command == "stop")
    {
      shutter.stop();
    }
    else if(command == "save")
    {
      saveConfig();
    }
    else if(command == "load")
    {
      loadConfig();
    }
    else if(command == "time")
    {
      Serial.println(shutter.getMoveTime());
    }
  }
  
}

// Init SPIFFS
void setupSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }
}

void loadConfig()
{
  jsonSpiffs.begin();
  jsonSpiffs.loadConfig();
  T_fullShutterMove = jsonSpiffs.get("T_fullShutterMove", DEFAULT_T_fullShutterMove);
  T_openingShutterMove = jsonSpiffs.get("T_openingShutterMove", DEFAULT_T_openingShutterMove);
  nModbusId = jsonSpiffs.get("nModbusId", DEFAULT_nModbusId);
  //Wifi ap settings
  String wifiSSID = jsonSpiffs.getNested<String>("wifiAP", "ssid");
  String wifiPassword = jsonSpiffs.getNested<String>("wifiAP", "password");

  jsonSpiffs.printConfig();
}

// //Loading config.json
// void loadConfig() {
//   File file = SPIFFS.open("/config.json", "r");
//   if (!file) {
//     Serial.println("Failed to open config file");
//     return;
//   }

//   JsonDocument doc;
//   DeserializationError error = deserializeJson(doc, file);
//   if (error) {
//     Serial.println("Failed to read config file, using default T_fullShutterMove");
//     T_fullShutterMove = DEFAULT_T_fullShutterMove;
//   } else {
//     T_fullShutterMove = doc["fullShutterMove"] | DEFAULT_T_fullShutterMove;
//     Serial.println("Loaded fullShutterMove: " + String(T_fullShutterMove));
//   }
//   file.close();
// }

// // Save data into config.json
// void saveConfig() {
//   File file = SPIFFS.open("/config.json", "w");
//   if (!file) {
//     Serial.println("Failed to open config file for writing");
//     return;
//   }

//   JsonDocument doc;
//   doc["fullShutterMove"] = T_fullShutterMove;
  
//   if (serializeJson(doc, file) == 0) {
//     Serial.println("Failed to write to config file");
//   }
  
//   file.close();
// }