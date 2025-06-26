#include "DebounceInterrupt.h"

DebounceInterrupt::DebounceInterrupt(uint8_t pin, uint8_t countTreshold, uint8_t edge)
  : pin(pin), countTreshold(countTreshold), pressed(false),
    pulseCount(0), lastPressedTime(0), notified(false), callback(nullptr){

    // Set the pin as input
    pinMode(pin, INPUT_PULLUP);
    
    debounceTimer = xTimerCreate(String("DebounceTmr"+String(pin)).c_str(), pdMS_TO_TICKS(TIMEOUT_LAST_PRESSED),
                                     pdFALSE, this, DebounceInterrupt::timerCallback);

    // Set the interrupt on the pin
    attachInterruptArg(digitalPinToInterrupt(pin), DebounceInterrupt::handleInterruptStatic, this, edge);
}

bool DebounceInterrupt::isPressed() const {
    return pressed;
}

void IRAM_ATTR DebounceInterrupt::handleInterruptStatic(void* arg) {
    DebounceInterrupt* self = static_cast<DebounceInterrupt*>(arg);
    self->handleInterrupt();
}

void IRAM_ATTR DebounceInterrupt::handleInterrupt() {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(millis() - lastPressedTime < NOISE_FILTER_TIME) {
        // If the interrupt was triggered too early, ignore it
        return;
    }

    // Check if enough time has passed since the last interrupt
    if(pulseCount == 0 || (millis() - lastPressedTime) < TIMEOUT_LAST_PRESSED) {
        pulseCount++;
        lastPressedTime = millis();
    } else {
        // If the time since the last press exceeds TIMEOUT_LAST_PRESSED, reset the count
        pulseCount = 0;
        lastPressedTime = millis();
    }

    if(xTimerIsTimerActive(debounceTimer)) {
        xTimerStopFromISR(debounceTimer, &xHigherPriorityTaskWoken);
    }

    if (pulseCount > countTreshold)
    {
        pressed = true;
        // Start a FreeRTOS timer to reset the pressed state
        xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);

        if (callback && !notified) {
            callback(pressed);  // Notify press
            notified = true;    // Mark as notified
        }

        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }

}

void DebounceInterrupt::timerCallback(TimerHandle_t xTimer) {
    DebounceInterrupt* self = static_cast<DebounceInterrupt*>(pvTimerGetTimerID(xTimer));

    self->pressed = false;  // Reset state

    if (self->callback) {
        self->callback(self->pressed);  // Notify state change
    }
    self->notified = false;
}

void DebounceInterrupt::setCallback(std::function<void(bool)> cb) {
    callback = cb;
}
