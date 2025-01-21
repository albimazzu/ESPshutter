#include "mbslave.h"

// // Inizializza i registri con valori predefiniti
uint16_t MbSlave::inputRegisters[4] = {0, 0, 300, 100}; // Status, Position, Fullmove Time, Collision Threshold
uint16_t MbSlave::holdingRegisters[2] = {CMD_STOP, 0};          // Command, Opening Percentage

MbSlave::MbSlave(HardwareSerial& serialPort, int txPin, int rxPin, int rtsPin)
    : serial(serialPort), txPin(txPin), rxPin(rxPin), rtsPin(rtsPin) {}

void MbSlave::begin(uint8_t slaveId) {
    // Configura la seriale e Modbus
    serial.begin(MODBUS_BAUDRATE, SERIAL_8N1, rxPin, txPin);
    modbus.begin(&serial, rtsPin);
    modbus.slave(slaveId);

    // Mappa i registri
    // modbus.addIreg(0, inputRegisters, 4);    // Input Registers (4 registri consecutivi)
    // modbus.addHreg(0, holdingRegisters, 2); // Holding Registers (2 registri consecutivi)

    // // Callback per scrittura sui holding registers
    // modbus.onSetHreg(writeHoldingRegister);
}


void MbSlave::task() {
    modbus.task();
}

uint16_t MbSlave::readInputRegister(Modbus::ResultCode event, uint16_t address) {
    if (address < 4) {
        return inputRegisters[address];
    }
    return 0;
}

bool MbSlave::writeHoldingRegister(Modbus::ResultCode event, uint16_t address, uint16_t value) {
    if (address < 2) {
        holdingRegisters[address] = value;
        return true;
    }
    return false;
}
