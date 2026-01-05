#ifndef MODBUSDEF_H
#define MODBUSDEF_H

#define MODBUS_BAUDRATE 9600

//Device modbus input registers
typedef enum {
    INPUTREG_STATUS = 0,
    INPUTREG_POSITION,
    INPUTREG_FULLMOVE_TIME,
    INPUTREG_COLLISION_THRESHOLD
} ModbusInputRegister;

//Device modbus holding registers
typedef enum {
    HOLDINGREG_COMMAND = 0,
    HOLDINGREG_TARGET_POSITION, //0-100% 0= closed, 100 = open
    HOLDINGREG_FULLMOVE_TIME_SET,
    HOLDINGREG_COLLISION_THRESHOLD_SET,
    HOLDINGREG_WRITE_CONFIG, //1= write config
    HOLDINGREG_TURN_ON_AP
} ModbusHoldingRegister;

typedef enum {
    CMD_NONE = 0,
    CMD_STOP,
    CMD_MOVE_UP,
    CMD_MOVE_DOWN,
    CMD_GO_TARGET
} eShutterCommand;

#endif //MODBUSDEF_H