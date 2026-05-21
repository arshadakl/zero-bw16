#include "log.h"
#include <stdarg.h>

Logger Log;

void Logger::init() {
    _head = 0;
    _count = 0;
    _changes = 0;
}

void Logger::log(LogLevel level, const char* fmt, ...) {
    LogEntry& e = _buf[_head];
    e.ts = millis();
    e.level = level;
    va_list args;
    va_start(args, fmt);
    vsnprintf(e.msg, sizeof(e.msg), fmt, args);
    va_end(args);
    Serial.printf("[%lu] %s\n", e.ts, e.msg);
    _head = (_head + 1) % MAX_LOG_ENTRIES;
    if (_count < MAX_LOG_ENTRIES) _count++;
    _changes++;
}

void Logger::clear() {
    _head = 0;
    _count = 0;
    _changes++;
}

const LogEntry* Logger::entries() const { return _buf; }
int Logger::count() const { return _count; }
uint32_t Logger::changeCounter() const { return _changes; }
