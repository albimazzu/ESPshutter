#ifndef MBSLAVE_H
#define MBSLAVE_H

#include <ModbusRTU.h>
#include <functional>
#include "ModbusDef.h"

class MbSlave {
public:
    MbSlave(HardwareSerial& serialPort, int rtsPin, int txPin=-1, int rxPin=-1);
    
    using HregCallback = std::function<uint16_t(uint16_t address, uint16_t value)>;

    void begin(unsigned long baud, uint8_t slaveId);
    void task(); // Da chiamare nel loop principale
    void onSetHreg(HregCallback callback);
    bool updateInputReg(uint16_t address, uint16_t value);
    uint16_t getHoldingReg(uint16_t address);
    bool writeHoldingReg(uint16_t address, uint16_t value);
  

private:
    ModbusRTU modbus;
    HardwareSerial& serial;
    int txPin;
    int rxPin;
    int rtsPin;

    static HregCallback hregCallback;
    static uint16_t hregCallbackHandler(TRegister* reg, uint16_t val);
};

#endif // MBSLAVE_H
