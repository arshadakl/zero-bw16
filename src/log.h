#pragma once
#include <Arduino.h>
#include "config.h"

enum LogLevel { LOG_INFO, LOG_WARN, LOG_ERROR, LOG_ATTACK, LOG_SNIFF };

struct LogEntry {
    uint32_t ts;
    LogLevel level;
    char msg[80];
};

class Logger {
public:
    void init();
    void log(LogLevel level, const char* fmt, ...);
    void clear();
    const LogEntry* entries() const;
    const LogEntry* lastEntry() const; // most recently written entry
    int count() const;
    uint32_t changeCounter() const;
private:
    LogEntry _buf[MAX_LOG_ENTRIES];
    int _head = 0;
    int _count = 0;
    uint32_t _changes = 0;
};

extern Logger Log;
