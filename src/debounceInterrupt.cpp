#include "DebounceInterrupt.h"

// Inizializza il puntatore statico
DebounceInterrupt* DebounceInterrupt::instances[4] = {nullptr, nullptr, nullptr, nullptr};

DebounceInterrupt::DebounceInterrupt(uint8_t timerIndex, uint8_t pin, uint32_t hzFreq)
  : timerIndex(timerIndex), pin(pin), hzFreq(hzFreq), pressed(false) {

    // Inizializza il timer hardware
    timer = timerBegin(timerIndex, TIMER_PRESCALER, true);
    timerAttachInterrupt(timer, &DebounceInterrupt::onTimerStatic, true);

    // Imposta il pin come input
    pinMode(pin, INPUT_PULLUP);

    // Imposta l'interrupt sul pin
    attachInterruptArg(digitalPinToInterrupt(pin), DebounceInterrupt::handleInterruptStatic, this, RISING);

    // Calcolo filterTime
    filterTime = 10e5/hzFreq;

    // Calcolo valore timeout
    timeout = 2*PULSE_COUNT_THRESHOLD*filterTime;

    // Registra l'istanza corrente nella mappa
    instances[timerIndex] = this;
}

bool DebounceInterrupt::isPressed() const {
    return pressed;
}

void IRAM_ATTR DebounceInterrupt::handleInterruptStatic(void* arg) {
    DebounceInterrupt* self = static_cast<DebounceInterrupt*>(arg);
    self->handleInterrupt();
}

void IRAM_ATTR DebounceInterrupt::onTimerStatic() {
    for (int i = 0; i < 4; ++i) {
        if (instances[i]) {
            instances[i]->onTimer();
        }
    }
}

void IRAM_ATTR DebounceInterrupt::handleInterrupt() {
    portENTER_CRITICAL_ISR(&timerMux); 


    // Verifica che sia trascorso filterTime dall'ultimo interrupt
    if(timerRead(timer) > filterTime)
        pulseCount++;

    if (pulseCount > PULSE_COUNT_THRESHOLD)
        pressed = true;


    timerAlarmDisable(timer);
    timerWrite(timer, 0);  
    timerAlarmWrite(timer, timeout, false);
    timerAlarmEnable(timer);
    portEXIT_CRITICAL_ISR(&timerMux);
}

//Timer timeout handler
void IRAM_ATTR DebounceInterrupt::onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    pulseCount = 0;
    pressed = false;
    timerAlarmDisable(timer);
    timerWrite(timer, 0);    
    portEXIT_CRITICAL_ISR(&timerMux);
}


uint32_t DebounceInterrupt::getFilterTime() {
    return filterTime;
}

uint32_t DebounceInterrupt::getTimeout() {
    return timeout;
}

