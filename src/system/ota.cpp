#include "ota.h"
#include "../log.h"

OtaHandler Ota;

bool OtaHandler::handleUpload(WiFiClient& client, uint32_t content_len) {
    // Realtek AmebaD OTA requires chip-specific flash management not
    // accessible via simple start/write/end API. Not implemented.
    (void)client;
    (void)content_len;
    Log.log(LOG_ERROR, "OTA: not supported in this build");
    return false;
}

uint32_t OtaHandler::bytesWritten() const { return _written; }
bool OtaHandler::inProgress() const { return _inProgress; }
