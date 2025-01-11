// Shutter.h
#ifndef SHUTTER_H
#define SHUTTER_H

#include <functional>
#include <Arduino.h>
#include "MillisTimer.h"

#define DEBUG 1
#define DEFAULT_MOVE_TIME 5000UL

typedef enum {
    INIT,
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
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

    // Timers
    MillisTimer TIMER_ShutterMove;
    MillisTimer TIMER_RelaySpikeFilter;

    // Callback for saving configuration
    std::function<void(unsigned long)> saveCallback;

public:
    Shutter(int upPin, int downPin, unsigned long shutterMoveTime=DEFAULT_MOVE_TIME);
    void begin(unsigned long shutterMoveTime=DEFAULT_MOVE_TIME);
    void commandUp(bool state);
    void commandDown(bool state);
    void stop();
    unsigned long getMoveTime();
    void handler();
    String shutterStateToString(eShutterState state);
    void log(String message);
};

#endif // SHUTTER_H