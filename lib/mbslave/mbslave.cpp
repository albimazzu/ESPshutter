#include "mbslave.h"

// Callback statica per i registri holding
MbSlave::HregCallback MbSlave::hregCallback = nullptr;

MbSlave::MbSlave(HardwareSerial& serialPort, int txPin, int rxPin, int rtsPin)
    : serial(serialPort), txPin(txPin), rxPin(rxPin), rtsPin(rtsPin) {}

void MbSlave::begin(unsigned long baud, uint8_t slaveId) {
    //Modbus initialization
    //serial.begin(baud, SERIAL_8N1, rxPin, txPin); TO FIX!!
    modbus.begin(&serial, rtsPin);
    modbus.slave(slaveId);
    
    //Mapping registers
    modbus.addIreg(0, 0, 10);
    modbus.addHreg(0, 0, 10);

    // Callback per scrittura sui holding registers
    //modbus.onSetHreg(0, hregCallbackHandler, 10);
}


void MbSlave::task() {
    modbus.task();
}

// Registra una callback globale per tutti i registri holding
void MbSlave::onSetHreg(HregCallback callback) {
    hregCallback = callback;
}

uint16_t MbSlave::hregCallbackHandler(TRegister* reg, uint16_t val) {
    if (hregCallback) {
        // Usa reinterpret_cast per ottenere l'indirizzo
        uint16_t address = *reinterpret_cast<uint16_t*>(&reg->address);
        return hregCallback(address, val);
    }
    // Ritorna il valore scritto come predefinito
    return val;
}

bool MbSlave::updateInputReg(uint16_t address, uint16_t value)
{
    return modbus.Ireg(address, value);
}

uint16_t MbSlave::getHoldingReg(uint16_t address) {
    return modbus.Hreg(address);
}

bool MbSlave::writeHoldingReg(uint16_t address, uint16_t value) {
    return modbus.Hreg(address, value);
}
