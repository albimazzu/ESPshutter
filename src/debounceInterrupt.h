#ifndef DEBOUNCEINTERRUPT_H
#define DEBOUNCEINTERRUPT_H

#include <Arduino.h>
#define TIMEOUT_LAST_PRESSED 100 // msec, max time between two interrupts
#define NOISE_FILTER_TIME 5 // msec, time to ignore noise

class DebounceInterrupt {
public:
    DebounceInterrupt(uint8_t pin, uint8_t countTreshold, uint8_t edge);
    bool isPressed() const;
    void setCallback(std::function<void(bool)> cb);

private:
    static void IRAM_ATTR handleInterruptStatic(void* arg);
    void IRAM_ATTR handleInterrupt();
    static void timerCallback(TimerHandle_t xTimer);

    uint8_t pin;
    uint8_t countTreshold;
    volatile uint8_t pulseCount;
    volatile uint32_t lastPressedTime;
    volatile bool pressed;
    volatile bool notified;
    
    TimerHandle_t debounceTimer;
    std::function<void(bool)> callback;
};

#endif // DEBOUNCEINTERRUPT_H
