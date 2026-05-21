#pragma once
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "../config.h"

class WebServer {
public:
    WebServer() : _srv(WEB_PORT) {}
    void init();
    void tick();    // call from loop()
    void pushSSE(const char* json);  // push to SSE client if connected
private:
    WiFiServer _srv;
    WiFiClient _sse;       // persistent SSE connection
    WiFiUDP    _dns;       // captive portal DNS
    bool       _dnsActive = false;

    uint32_t _lastSSE = 0;
    uint32_t _lastStats = 0;

    void _handleClient(WiFiClient& c);
    bool _parseLine(WiFiClient& c, char* buf, int maxlen);
    void _serveIndex(WiFiClient& c);
    void _serveSSE(WiFiClient& c);
    void _serve404(WiFiClient& c);

    void _startDns();
    void _tickDns();
    void _tickSSE();
};

extern WebServer WebSrv;
