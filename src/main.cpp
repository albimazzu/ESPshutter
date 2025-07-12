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

long T_fullShutterMove;
long T_openingShutterMove;
int nModbusId;
String wifiApSSID;
String wifiApPassword;

JsonSpiffs jsonSpiffs("/config.json");

Connection wifiConnection;

MbSlave modbusSlave(Serial0, PIN_485DIR);

// Istanza della classe DebounceInterrupt
DebounceInterrupt upCommand(PIN_ACIN_1, 4, FALLING);
DebounceInterrupt downCommand(PIN_ACIN_2, 4, FALLING);
// DebounceInterrupt remoteUpCommand(PIN_ACIN_3, 4, FALLING);
// DebounceInterrupt remoteDownCommand(PIN_ACIN_4, 4, FALLING);

bool lastUpCommand = false;
bool lastDownCommand = false;

MillisTimer TIMER_Heartbeat;
MillisTimer TIMER_BtnLongPress;

Shutter shutter(PIN_UP_CMD, PIN_DOWN_CMD);

void physicalInputsHandler();
void modbusCommandsHandler();
void heartbeat();
void debug();

//SPIFFS functions
void setupSPIFFS();
void loadConfig();
void saveConfig();


// void eventoTasto1(bool pressed) {
//     Serial.println("[Tasto 1] Stato: " + String(pressed));
// }

// void eventoTasto2(bool pressed) {
//     Serial.println("[Tasto 2] Stato: " + String(pressed));
// }

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

  TIMER_Heartbeat.begin(500);  
  TIMER_BtnLongPress.begin(1500);

  shutter.begin(T_fullShutterMove);

  //turn on wifi on startup
  wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);

  // upCommand.setCallback(eventoTasto1);
  // downCommand.setCallback(eventoTasto2);

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
  
  //Turn on AP if USER button is pressed
  if(!digitalRead(PIN_USRBTN) && wifiConnection.getWiFiStatus() == WIFI_OFF)
      wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);
      
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
      shutter.stop();
    else
      shutter.commandUp();
  }
  //FALLING edge detection upCommand
  if(upCommand.isPressed() == false && lastUpCommand == true)
  {
    Serial.println("upCommand falling edge");
    if(TIMER_BtnLongPress.fire())
    {
      TIMER_BtnLongPress.stop();
      shutter.stop();
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
      shutter.stop();
    else
      shutter.commandDown();
  }
  //FALLING edge detection downCommand
  if(downCommand.isPressed() == false && lastDownCommand == true)
  {
    Serial.println("downCommand falling edge");    
    if(TIMER_BtnLongPress.fire())
    {
      TIMER_BtnLongPress.stop();
      shutter.stop();
    }
  }
  lastDownCommand = downCommand.isPressed();
  #pragma endregion      

  if(upCommand.isPressed() || downCommand.isPressed())
    TIMER_BtnLongPress.start();
  else
    TIMER_BtnLongPress.stop();
}

void modbusCommandsHandler()
{
  if(modbusSlave.getHoldingReg(HOLDINGREG_COMMAND) > 0)
  {
    //Command changed
    int command = modbusSlave.getHoldingReg(HOLDINGREG_COMMAND);
    modbusSlave.writeHoldingReg(HOLDINGREG_COMMAND, 0); //reset command register
    Serial.println("Received modbus command: " + String(command));
    switch (command)
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
    
    default:
      break;
    }
  }

  if(modbusSlave.getHoldingReg(HOLDINGREG_TURN_ON_AP) > 0)
  {
    //Turn on AP command received
    Serial.println("Received modbus command: TURN ON AP");
    modbusSlave.writeHoldingReg(HOLDINGREG_TURN_ON_AP, 0); //reset turn on AP command
    if(wifiConnection.getWiFiStatus() == WIFI_OFF)
      wifiConnection.initWiFiAP(wifiApSSID.c_str(), wifiApPassword.c_str(), 60000);
    
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
