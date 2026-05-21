#pragma once
#include <Arduino.h>

enum ButtonEvent { BTN_NONE, BTN_SHORT, BTN_LONG };

class ButtonController {
public:
    void init(int pin, bool activeLow = true);
    ButtonEvent poll();
private:
    int _pin = -1;
    bool _activeLow = true;
    bool _lastState = false;
    bool _pressed = false;
    uint32_t _pressTime = 0;
    static constexpr uint32_t LONG_PRESS_MS = 1000;
    static constexpr uint32_t DEBOUNCE_MS   = 50;
    uint32_t _lastChange = 0;
};

extern ButtonController Button;
