#pragma once
#include <Arduino.h>

enum LedState { LED_OFF, LED_IDLE, LED_SCAN, LED_ATTACK, LED_ERROR };

class LedController {
public:
    void init(int pin, bool activeLow = true);
    void setState(LedState s);
    void setEnabled(bool en);
    bool isEnabled() const;
    void tick();
private:
    int _pin = -1;
    bool _activeLow = true;
    bool _enabled = true;
    LedState _state = LED_IDLE;
    uint32_t _lastToggle = 0;
    bool _on = false;
    uint8_t _blinkCount = 0;
    uint8_t _blinkPhase = 0;
    void _write(bool on);
};

extern LedController Led;
