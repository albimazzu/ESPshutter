#include <Arduino.h>
#include "SPIFFS.h"
#include <FS.h>
#include <ArduinoJson.h> // JSON parsing
#include <Preferences.h>
#include "mapping.h"
#include "debounceInterrupt.h"
#include "MillisTimer.h"
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

//MbSlave modbusSlave(Serial1, PIN_485DIR, 21, 20);
MbSlave modbusSlave(Serial0, PIN_485DIR);

// Istanza della classe DebounceInterrupt
DebounceInterrupt upCommand(0, PIN_ACIN_1, 60); //freq in Hz
DebounceInterrupt downCommand(1, PIN_ACIN_2, 60); //freq in Hz
DebounceInterrupt remoteUpCommand(2, PIN_ACIN_3, 60); //freq in Hz
DebounceInterrupt remoteDownCommand(3, PIN_ACIN_4, 60); //freq in Hz
bool lastUpCommand = false;
bool lastDownCommand = false;
bool lastRemoteUpCommand = false;
bool lastRemoteDownCommand = false;

MillisTimer TIMER_Heartbeat;
MillisTimer TIMER_CalibrationTrigger;

Shutter shutter(PIN_UP_CMD, PIN_DOWN_CMD);

uint16_t oldModbusCommand = 0;

void physicalInputsHandler();
void modbusCommandsHandler();
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
  //pinMode(20, INPUT_PULLDOWN); //weak pulldown on RX pin to allow correct signal level, needed if R34 is missing

  loadConfig();

  modbusSlave.begin(MODBUS_BAUDRATE, nModbusId);

  // Serial0.begin(115200);
  // digitalWrite(PIN_485DIR, LOW);
  // while (1)
  // {
  //   if(Serial0.available() > 0)
  //   {
  //     Serial.print(Serial0.read());
  //   }
  // }


  TIMER_Heartbeat.begin(500);  
  TIMER_CalibrationTrigger.begin(DEF_CALIBRATION_PRESSED_TIME);

  shutter.begin(T_fullShutterMove);

  //turn on wifi on startup
  wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);

  delay(1000);
  Serial.println("Setup completed");
  
}


void loop() {

  heartbeat();
  shutter.handler();
  physicalInputsHandler();
  modbusSlave.task();
  modbusCommandsHandler();


  //Update input reg
  modbusSlave.updateInputReg(INPUTREG_STATUS, shutter.getShutterState());
  // modbusSlave.updateInputReg(INPUTREG_POSITION, shutter.getPosition());
  modbusSlave.updateInputReg(INPUTREG_FULLMOVE_TIME, shutter.getFullMoveTime());
  // modbusSlave.updateInputReg(INPUTREG_COLLISION_THRESHOLD, shutter.getCollisionThreshold());
  

  if(!digitalRead(PIN_USRBTN) && wifiConnection.getWiFiStatus() == WIFI_OFF)
  {
   wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);
  }
  wifiConnection.loop();
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
    if(shutter.getFullMoveTime() != T_fullShutterMove)
    {
      T_fullShutterMove = shutter.getFullMoveTime();
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
    if(shutter.getFullMoveTime() != T_fullShutterMove)
    {
      T_fullShutterMove = shutter.getFullMoveTime();
      jsonSpiffs.set("T_fullShutterMove", T_fullShutterMove);
      jsonSpiffs.saveConfig();
      Serial.println("New calibration time:"+String(T_fullShutterMove));
    }
  }
  lastDownCommand = downCommand.isPressed();
  #pragma endregion
  
  #pragma region remoteUpCommand
  //RISING edge detection remoteUpCommand
  if(remoteUpCommand.isPressed() == true && lastRemoteUpCommand == false)
  {
    Serial.println("remoteUpCommand rising edge");
    if(shutter.isMoving())
    {
      shutter.stop();
      shutter.commandUp();
    }
    else
    {
      shutter.commandUp();
    }
  }
  //FALLING edge detection remoteUpCommand
  if(upCommand.isPressed() == false && lastUpCommand == true)
  {
    Serial.println("remoteUpCommand falling edge");
    shutter.stop();   
  }
  lastRemoteUpCommand = remoteUpCommand.isPressed();
  #pragma endregion

  #pragma region remoteDownCommand
  //RISING edge detection remoteDownCommand
  if(downCommand.isPressed() == true && lastDownCommand == false)
  {
    Serial.println("downCommand rising edge");
    if(shutter.isMoving())
    {
      shutter.stop();
      shutter.commandDown();
    }
    else
    {
      shutter.commandDown();
    }
  }
  //FALLING edge detection remoteDownCommand
  if(downCommand.isPressed() == false && lastDownCommand == true)
  {
    Serial.println("downCommand falling edge");
    shutter.stop();
  }
  lastRemoteDownCommand = remoteDownCommand.isPressed();
  #pragma endregion
  

  if(TIMER_CalibrationTrigger.fire())
  {
    TIMER_CalibrationTrigger.stop();
    if(shutter.isMoving())
      shutter.startCalibration();
  }
}

void modbusCommandsHandler()
{
  if(modbusSlave.getHoldingReg(HOLDINGREG_COMMAND) != oldModbusCommand)
  {
    //Command changed
    oldModbusCommand = modbusSlave.getHoldingReg(HOLDINGREG_COMMAND);
    Serial.println("Received modbus command: " + String(oldModbusCommand));
    switch (oldModbusCommand)
    {
    case CMD_STOP:
      if(shutter.isMoving() && !shutter.isCalibrating())
        shutter.stop();
      break;

    case CMD_MOVE_UP:
      if(!shutter.isCalibrating())
      {
        if(shutter.isMovingDown())
          shutter.stop();
        shutter.commandUp();
      }
      break;

    case CMD_MOVE_DOWN:
      if(!shutter.isCalibrating())
      {
        if(shutter.isMovingUp())
          shutter.stop();
        shutter.commandDown();
      }
      break;

    case CMD_GO_TARGET:
      /* code */
      break;

    case 10:
      wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);
      break;
    
    default:
      break;
    }
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
      Serial.println(shutter.getFullMoveTime());
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
  bool configExist = jsonSpiffs.loadConfig();

  T_fullShutterMove = jsonSpiffs.get("T_fullShutterMove", DEFAULT_T_fullShutterMove);
  T_openingShutterMove = jsonSpiffs.get("T_openingShutterMove", DEFAULT_T_openingShutterMove);
  nModbusId = jsonSpiffs.get("nModbusId", DEFAULT_nModbusId);
  //Wifi ap settings
  wifiApSSID = jsonSpiffs.getNested<String>("wifiAP", "ssid", DEFAULT_AP_SSID);
  wifiApPassword = jsonSpiffs.getNested<String>("wifiAP", "password", DEFAULT_AP_PASSWORD);

  if(!configExist)
    jsonSpiffs.saveConfig();

  jsonSpiffs.printConfig();
}
