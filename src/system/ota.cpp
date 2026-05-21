#include "ota.h"
#include "../log.h"

// Realtek AmebaD OTA API
extern "C" {
    extern int ota_update_start(void);
    extern int ota_update_write(uint8_t* buf, uint32_t len);
    extern int ota_update_end(void);
    extern void sys_reset(void);
}

OtaHandler Ota;

bool OtaHandler::handleUpload(WiFiClient& client, uint32_t content_len) {
    if (content_len == 0 || content_len > 1024 * 1024) {
        Log.log(LOG_ERROR, "OTA: invalid size %u", content_len);
        return false;
    }
    _inProgress = true;
    _written = 0;
    Log.log(LOG_INFO, "OTA: starting, size=%u", content_len);

    if (ota_update_start() != 0) {
        Log.log(LOG_ERROR, "OTA: start failed");
        _inProgress = false;
        return false;
    }

    uint8_t buf[OTA_CHUNK_SIZE];
    uint32_t remaining = content_len;
    uint32_t timeout = millis() + 30000;

    while (remaining > 0 && millis() < timeout) {
        uint32_t toRead = min((uint32_t)sizeof(buf), remaining);
        int got = client.readBytes(buf, toRead);
        if (got <= 0) { delay(5); continue; }
        if (ota_update_write(buf, got) != 0) {
            Log.log(LOG_ERROR, "OTA: write failed at %u", _written);
            _inProgress = false;
            return false;
        }
        _written += got;
        remaining -= got;
        timeout = millis() + 30000;
    }

    if (remaining > 0) {
        Log.log(LOG_ERROR, "OTA: timeout, wrote %u/%u", _written, content_len);
        _inProgress = false;
        return false;
    }

    if (ota_update_end() != 0) {
        Log.log(LOG_ERROR, "OTA: finalize failed");
        _inProgress = false;
        return false;
    }

    Log.log(LOG_INFO, "OTA: complete, rebooting");
    _inProgress = false;
    delay(500);
    sys_reset();
    return true;
}

uint32_t OtaHandler::bytesWritten() const { return _written; }
bool OtaHandler::inProgress() const { return _inProgress; }
