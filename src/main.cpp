#include <Arduino.h>
#include "SPIFFS.h"
#include <FS.h>
#include <ArduinoJson.h> // JSON parsing

#include "mapping.h"
#include "debounceInterrupt.h"
#include "MillisTimer.h"
#include "secrets.h"
#include "connection.h"
#include "shutter.h"

#define DEFAULT_TIME_fullShutterMove 10000
#define CALIBRATION_PRESSED_TIME 5000 //msec, when up/down buttons are pressed more than this time start calibration


Connection wifiConnection;

// Istanza della classe DebounceInterrupt
DebounceInterrupt debounceInterruptUp(0, PIN_ACIN_1, 60); //freq in Hz
DebounceInterrupt debounceInterruptDown(1, PIN_ACIN_2, 60); //freq in Hz

MillisTimer TIMER_Heartbeat;

Shutter shutter(PIN_UP_CMD, PIN_DOWN_CMD);

unsigned long TIME_fullShutterMove = 0;

void heartbeat();

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

  TIMER_Heartbeat.begin(500);  

  setupSPIFFS();
  loadConfig();  // Load config.json var values

  shutter.begin(TIME_fullShutterMove);

  //turn on wifi on startup
  //wifiConnection.initWiFiAP("ESP_shutter", "12345678", 60000);

  delay(1000);
  Serial.println("Setup completed");
  
}


void loop() {

  heartbeat();
  shutter.handler();
  
  if(Serial.available()>0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if(command == "up")
    {
      shutter.commandUp(true);
    }
    else if(command == "down")
    {
      shutter.commandDown(true);
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
    else if(command == "calibrate")
    {
      shutter.commandUp(true);
      delay(CALIBRATION_PRESSED_TIME);
      shutter.stop();
      shutter.commandDown(true);
      delay(CALIBRATION_PRESSED_TIME);
      shutter.stop();
    }
  }

  // if(!digitalRead(PIN_USRBTN) && wifiConnection.getWiFiStatus() == WIFI_OFF)
  // {
  //   wifiConnection.initWiFiAP("ESP_shutter", "12345678", 60000);
  // }
  // wifiConnection.loop();
}

void heartbeat()
{
  TIMER_Heartbeat.start();
  if(TIMER_Heartbeat.fire(true))
  {
    digitalWrite(PIN_STATUSLED,!digitalRead(PIN_STATUSLED));
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

//Loading config.json
void loadConfig() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read config file, using default TIME_fullShutterMove");
    TIME_fullShutterMove = DEFAULT_TIME_fullShutterMove;
  } else {
    TIME_fullShutterMove = doc["TIME_fullShutterMove"] | DEFAULT_TIME_fullShutterMove;
    Serial.println("Loaded TIME_fullShutterMove: " + String(TIME_fullShutterMove));
  }
  file.close();
}

// Save data into config.json
void saveConfig() {
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  JsonDocument doc;
  doc["TIME_fullShutterMove"] = TIME_fullShutterMove;
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to config file");
  }
  
  file.close();
}