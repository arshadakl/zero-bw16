#pragma once
#include <WiFiClient.h>

class ApiHandler {
public:
    // Returns true if request was handled
    bool handle(WiFiClient& client, const char* method, const char* path,
                const char* body, int body_len);
private:
    void _respond(WiFiClient& c, int code, const char* ct, const char* body);
    void _respondJson(WiFiClient& c, const char* json);
    void _respondOk(WiFiClient& c);
    void _respondErr(WiFiClient& c, const char* msg);

    // Endpoints
    void _getScan(WiFiClient& c);
    void _postScan(WiFiClient& c);
    void _getNetworks(WiFiClient& c);
    void _postAttackStart(WiFiClient& c, const char* body, int len);
    void _postAttackStop(WiFiClient& c);
    void _getAttackStats(WiFiClient& c);
    void _postSniffStart(WiFiClient& c);
    void _postSniffStop(WiFiClient& c);
    void _getSniffProbes(WiFiClient& c);
    void _getSniffClients(WiFiClient& c, const char* query);
    void _getSniffPMKIDs(WiFiClient& c);
    void _getSniffHandshakes(WiFiClient& c);
    void _postSniffClearProbes(WiFiClient& c);
    void _postSniffClearClients(WiFiClient& c);
    void _postBeaconStart(WiFiClient& c, const char* body, int len);
    void _postBeaconStop(WiFiClient& c);
    void _getLog(WiFiClient& c);
    void _postLogClear(WiFiClient& c);
    void _getSettings(WiFiClient& c);
    void _postSettings(WiFiClient& c, const char* body, int len);
    void _postSettingsReset(WiFiClient& c);
    void _postOta(WiFiClient& c, int content_len);

    // Tiny JSON helpers
    static void _bssidStr(const uint8_t* b, char* out);
    static void _hexStr(const uint8_t* b, int len, char* out);
    static int  _jsonInt(const char* body, const char* key, int def);
    static bool _jsonStr(const char* body, const char* key, char* out, int maxlen);
    static bool _jsonBool(const char* body, const char* key, bool def);
};

extern ApiHandler Api;
