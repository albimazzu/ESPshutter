#ifndef DEBOUNCEINTERRUPT_H
#define DEBOUNCEINTERRUPT_H

#include <Arduino.h>
#define TIMER_PRESCALER 80
#define PULSE_COUNT_THRESHOLD 2

class DebounceInterrupt {
public:
    DebounceInterrupt(uint8_t timerIndex, uint8_t pin, uint32_t hzFreq);
    bool isPressed() const;
    uint32_t getTimeout();
    uint32_t getFilterTime();

private:
    static void IRAM_ATTR handleInterruptStatic(void* arg);
    static void IRAM_ATTR onTimerStatic();
    void IRAM_ATTR handleInterrupt();
    void IRAM_ATTR onTimer();

    uint8_t timerIndex;
    uint8_t pin;
    uint32_t hzFreq;
    volatile int pulseCount;
    volatile bool pressed;
    uint32_t filterTime;
    uint32_t timeout;
    hw_timer_t* timer;
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
    volatile uint32_t lastInterruptTime; // Variabile per memorizzare il tempo dell'ultimo interrupt


    static DebounceInterrupt* instances[4]; // Massimo 4 timer hardware
};

#endif // DEBOUNCEINTERRUPT_H
