// Shutter.h
#ifndef SHUTTER_H
#define SHUTTER_H

#include <functional>
#include <Arduino.h>
#include "MillisTimer.h"

#define DEBUG 1
#define DEFAULT_MOVE_TIME 5000UL
#define CALIBRATION_TIMEOUT_TIME 60000UL

typedef enum {
    INIT,
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
    CALIBRATE,
    COLLISION,
    STOP
} eShutterState;

class Shutter {
private:
    // Pins
    int pinUp;
    int pinDown;

    // State machine variables
    eShutterState shutterState;
    eShutterState oldShutterState;

    // Timing variables
    unsigned long moveTime;
    unsigned long elapsedMoveTime; //used when calibrating

    // Timers
    MillisTimer TIMER_ShutterMove;
    MillisTimer TIMER_CalibrationTimeout;

    // Callback for saving configuration
    std::function<void(unsigned long)> saveCallback;

public:
    Shutter(int upPin, int downPin, unsigned long shutterMoveTime=DEFAULT_MOVE_TIME);
    void begin(unsigned long shutterMoveTime=DEFAULT_MOVE_TIME);
    void commandUp();
    void commandDown();
    void startCalibration();
    void stopCalibration();
    void stop();
    bool isMoving();
    bool isMovingUp();
    bool isMovingDown();
    bool isCalibrating();
    unsigned long getFullMoveTime();
    unsigned long getLastMoveTime();
    void handler();
    String shutterStateToString(eShutterState state);
    eShutterState getShutterState();
    void log(String message);
};

#endif // SHUTTER_H