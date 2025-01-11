#ifndef DEF_H
#define DEF_H

typedef enum {
    STOPPED,
    MOVING_UP,
    MOVING_DOWN,
    COLLISION,
    CALIBRATION
} ShutterState;

typedef enum {
    STOP,
    MOVEUP,
    MOVEDOWN,
    GOTARGET,
    CALIBRATE
} ShutterCommand;

#endif //DEF_H