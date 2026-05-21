#pragma once
#include <Arduino.h>

enum LedState { LED_OFF, LED_IDLE, LED_SCAN, LED_ATTACK, LED_ERROR };

class LedController {
public:
    // pinGreen = -1 means no green LED; attack state uses 2 Hz blink on primary LED
    void init(int pinRed, int pinGreen = -1, bool activeLow = true);
    void setState(LedState s);
    void setEnabled(bool en);
    bool isEnabled() const;
    void tick();
private:
    int  _pin      = -1;
    int  _pinGreen = -1;
    bool _activeLow = true;
    bool _enabled   = true;
    LedState _state = LED_IDLE;
    uint32_t _lastToggle = 0;
    bool _on = false;
    uint8_t _blinkPhase = 0;
    void _write(bool red, bool green);
};

extern LedController Led;
