#include "led.h"

LedController Led;

void LedController::init(int pinRed, int pinGreen, bool activeLow) {
    _pin       = pinRed;
    _pinGreen  = pinGreen;
    _activeLow = activeLow;
    if (_pin >= 0)      { pinMode(_pin,      OUTPUT); }
    if (_pinGreen >= 0) { pinMode(_pinGreen, OUTPUT); }
    _write(false, false);
}

void LedController::setState(LedState s) {
    _state      = s;
    _blinkPhase = 0;
    // When switching away from ATTACK, turn green off immediately
    if (s != LED_ATTACK && _pinGreen >= 0)
        digitalWrite(_pinGreen, _activeLow ? HIGH : LOW);
}

void LedController::setEnabled(bool en) {
    _enabled = en;
    if (!en) _write(false, false);
}

bool LedController::isEnabled() const { return _enabled; }

void LedController::_write(bool red, bool green) {
    _on = red;
    if (_pin >= 0)
        digitalWrite(_pin, _activeLow ? !red : red);
    if (_pinGreen >= 0)
        digitalWrite(_pinGreen, _activeLow ? !green : green);
}

void LedController::tick() {
    if (!_enabled) { _write(false, false); return; }
    if (_pin < 0)  return;

    uint32_t now = millis();
    bool hasGreen = (_pinGreen >= 0);

    switch (_state) {
    case LED_OFF:
        _write(false, false);
        break;

    case LED_IDLE:
        // Blue slow blink 1 Hz
        if (now - _lastToggle >= 1000) {
            _lastToggle = now;
            _on = !_on;
            _write(_on, false);
        }
        break;

    case LED_SCAN:
        // Both LEDs fast blink 8 Hz
        if (now - _lastToggle >= 125) {
            _lastToggle = now;
            _on = !_on;
            _write(_on, hasGreen ? _on : false);
        }
        break;

    case LED_ATTACK:
        if (hasGreen) {
            // Green solid, red off
            _write(false, true);
        } else {
            // No green LED — 2 Hz blink on primary (clearly different from 1 Hz idle)
            if (now - _lastToggle >= 500) {
                _lastToggle = now;
                _write(!_on, false);
            }
        }
        break;

    case LED_ERROR:
        // Red double-blink then long pause
        if (now - _lastToggle >= 150) {
            _lastToggle = now;
            _blinkPhase++;
            if (_blinkPhase == 1 || _blinkPhase == 3) _write(true,  false);
            else if (_blinkPhase == 2 || _blinkPhase == 4) _write(false, false);
            else if (_blinkPhase >= 5) {
                _blinkPhase  = 0;
                _lastToggle = now + 700; // long pause before repeating
            }
        }
        break;
    }
}
