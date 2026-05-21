#include "led.h"

LedController Led;

void LedController::init(int pin, bool activeLow) {
    _pin = pin;
    _activeLow = activeLow;
    pinMode(_pin, OUTPUT);
    _write(false);
}

void LedController::setState(LedState s) {
    _state = s;
    _blinkPhase = 0;
    _blinkCount = 0;
}

void LedController::setEnabled(bool en) {
    _enabled = en;
    if (!en) _write(false);
}

bool LedController::isEnabled() const { return _enabled; }

void LedController::_write(bool on) {
    if (_pin < 0) return;
    _on = on;
    digitalWrite(_pin, _activeLow ? !on : on);
}

void LedController::tick() {
    if (!_enabled || _pin < 0) return;
    uint32_t now = millis();

    switch (_state) {
    case LED_OFF:
        _write(false);
        break;
    case LED_IDLE:
        // slow blink 1Hz
        if (now - _lastToggle >= 1000) { _lastToggle = now; _write(!_on); }
        break;
    case LED_SCAN:
        // fast blink 8Hz
        if (now - _lastToggle >= 125) { _lastToggle = now; _write(!_on); }
        break;
    case LED_ATTACK:
        // solid on
        _write(true);
        break;
    case LED_ERROR:
        // double blink: on-off-on-off-pause
        if (now - _lastToggle >= 150) {
            _lastToggle = now;
            _blinkPhase++;
            if (_blinkPhase == 1 || _blinkPhase == 3) _write(true);
            else if (_blinkPhase == 2 || _blinkPhase == 4) _write(false);
            else if (_blinkPhase >= 5) { _blinkPhase = 0; _lastToggle = now + 700; }
        }
        break;
    }
}
