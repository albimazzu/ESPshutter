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
    HOLDINGREG_OPENING_PERCENTAGE
} ModbusHoldingRegister;

typedef enum {
    CMD_NONE = 0,
    CMD_STOP,
    CMD_MOVE_UP,
    CMD_MOVE_DOWN,
    CMD_GO_TARGET
} eShutterCommand;

#endif //MODBUSDEF_H