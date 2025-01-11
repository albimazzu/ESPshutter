#include "Shutter.h"

// Constructor for Shutter class
// Parameters:
// - upPin: GPIO pin number for the up direction
// - downPin: GPIO pin number for the down direction
// - shutterMoveTime: Time in milliseconds for a full shutter movement
Shutter::Shutter(int upPin, int downPin, unsigned long shutterMoveTime)
    : pinUp(upPin),
      pinDown(downPin),
      moveTime(shutterMoveTime),
      shutterState(INIT),
      oldShutterState(IDLE){}

// Initializes the shutter pins and sets them to LOW
void Shutter::begin(unsigned long shutterMoveTime) {
    pinMode(pinUp, OUTPUT);
    pinMode(pinDown, OUTPUT);
    digitalWrite(pinUp, LOW);
    digitalWrite(pinDown, LOW);
    moveTime = shutterMoveTime;
    TIMER_ShutterMove.begin(moveTime);
    shutterState = INIT;
}

// Commands the shutter to move up if it is in the IDLE state
// Parameters:
// - state: Boolean value to command the up movement
void Shutter::commandUp(bool state) {
    if(!state)
        return;
        
    if (shutterState == IDLE) {
        digitalWrite(pinUp, HIGH);
        shutterState = MOVING_UP;
    }
}

// Commands the shutter to move down if it is in the IDLE state
// Parameters:
// - state: Boolean value to command the down movement
void Shutter::commandDown(bool state) {
    if(!state)
        return;

    if (shutterState == IDLE) {
        digitalWrite(pinDown, HIGH);
        shutterState = MOVING_DOWN;
    }
}

// Stops the shutter movement if it is currently moving
void Shutter::stop() {
    if (shutterState == MOVING_UP || shutterState == MOVING_DOWN) {
        shutterState = STOP;
        log("Shutter stopped");
    }
}

// Returns the elapsed move time of the shutter
// Returns:
// - Elapsed time in milliseconds since the shutter started moving
unsigned long Shutter::getMoveTime() {
    return TIMER_ShutterMove.getElapsedTime();
}

// Handles the shutter state transitions and performs actions based on the current state
void Shutter::handler() {
    if (shutterState != oldShutterState) {
        oldShutterState = shutterState;
        log("shutterState=" + shutterStateToString(shutterState));
    }

    switch (shutterState) {
        case INIT:
            digitalWrite(pinUp, LOW);
            digitalWrite(pinDown, LOW);
            TIMER_ShutterMove.stop();
            shutterState = IDLE;
            break;
        
        case IDLE:
            // Waiting for command triggered by commandUp or commandDown
            break;

        case MOVING_UP:
            TIMER_ShutterMove.start();
            if(TIMER_ShutterMove.fire()){
                shutterState = STOP;
            }
            break;

        case MOVING_DOWN:
            TIMER_ShutterMove.start();
            if(TIMER_ShutterMove.fire()){
                shutterState = STOP;
            }
            break;

        case STOP:
            TIMER_ShutterMove.stop();
            digitalWrite(pinUp, false);
            digitalWrite(pinDown, false);
            shutterState = INIT;          
            break;

        default:
           log("invalid shutterState!");
            shutterState = INIT;
            break;
    }
}

// Converts the shutter state enum to a string
// Parameters:
// - state: Shutter state enum value
// Returns:
// - String representation of the shutter state
String Shutter::shutterStateToString(eShutterState state) {
    switch (state) {
        case INIT:
            return "INIT";
        case IDLE:
            return "IDLE";
        case MOVING_UP:
            return "MOVING_UP";
        case MOVING_DOWN:
            return "MOVING_DOWN";
        case COLLISION:
            return "COLLISION";
        case STOP:
            return "STOP";
        default:
            return "UNKNOWN";
    }
}

void Shutter::log(String message) {
    if (DEBUG) {
        Serial.println(message);
    }
}