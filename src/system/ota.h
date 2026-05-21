#pragma once
#include <Arduino.h>
#include <WiFiClient.h>

class OtaHandler {
public:
    // Call from web server when /ota POST arrives with binary data
    // content_len: total firmware bytes, client: open WiFiClient for reading
    bool handleUpload(WiFiClient& client, uint32_t content_len);
    uint32_t bytesWritten() const;
    bool inProgress() const;
private:
    bool _inProgress = false;
    uint32_t _written = 0;
};

extern OtaHandler Ota;
