#include "button.h"

ButtonController Button;

void ButtonController::init(int pin, bool activeLow) {
    _pin = pin;
    _activeLow = activeLow;
    pinMode(_pin, INPUT_PULLUP);
    _lastState = digitalRead(_pin);
}

ButtonEvent ButtonController::poll() {
    if (_pin < 0) return BTN_NONE;
    uint32_t now = millis();
    bool raw = digitalRead(_pin);
    bool active = _activeLow ? !raw : raw;

    if (active != _pressed && now - _lastChange > DEBOUNCE_MS) {
        _lastChange = now;
        _pressed = active;
        if (active) {
            _pressTime = now;
        } else {
            uint32_t dur = now - _pressTime;
            if (dur >= LONG_PRESS_MS) return BTN_LONG;
            if (dur >= DEBOUNCE_MS)   return BTN_SHORT;
        }
    }
    return BTN_NONE;
}
