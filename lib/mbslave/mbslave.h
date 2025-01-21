#ifndef MBSLAVE_H
#define MBSLAVE_H

#include <ModbusRTU.h>
#include "ModbusDef.h"

class MbSlave {
public:
    MbSlave(HardwareSerial& serialPort, int txPin, int rxPin, int rtsPin);

    void begin(uint8_t slaveId);
    void task(); // Da chiamare nel loop principale

    // Gestione dei registri
    static uint16_t readInputRegister(Modbus::ResultCode event, uint16_t address);
    static bool writeHoldingRegister(Modbus::ResultCode event, uint16_t address, uint16_t value);

private:
    ModbusRTU modbus;
    HardwareSerial& serial;
    int txPin;
    int rxPin;
    int rtsPin;

    // Registri
    static uint16_t inputRegisters[4];
    static uint16_t holdingRegisters[2];
};

#endif // MBSLAVE_H
