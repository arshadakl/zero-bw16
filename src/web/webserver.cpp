#include "webserver.h"
#include "pages.h"
#include "api.h"
#include "../wifi/attacker.h"
#include "../log.h"
#include <string.h>
#include <stdio.h>

WebServer WebSrv;

void WebServer::init() {
    _srv.begin(WEB_PORT);
    _startDns();
    Log.log(LOG_INFO, "WebServer on port %d", WEB_PORT);
}

void WebServer::_startDns() {
    if (_dns.begin(53)) {
        _dnsActive = true;
        Log.log(LOG_INFO, "DNS captive portal active");
    }
}

void WebServer::tick() {
    // Accept new HTTP client
    WiFiClient c = _srv.available();
    if (c) _handleClient(c);

    // Push SSE stats every second
    _tickSSE();

    // Handle DNS (captive portal)
    if (_dnsActive) _tickDns();
}

void WebServer::_tickDns() {
    int pkt = _dns.parsePacket();
    if (pkt < 12) return;
    uint8_t buf[128];
    int len = _dns.read(buf, sizeof(buf));
    if (len < 12) return;

    // Build minimal DNS A response pointing to 192.168.1.1
    uint8_t resp[128];
    memcpy(resp, buf, min(len, 12)); // copy header
    resp[2] = 0x81; resp[3] = 0x80; // QR=1 Opcode=0 AA=1 RCODE=0
    resp[4] = resp[5]; resp[5] = resp[5]; // QDCOUNT same
    resp[6] = 0; resp[7] = 1;            // ANCOUNT = 1
    resp[8] = 0; resp[9] = 0;            // NSCOUNT
    resp[10] = 0; resp[11] = 0;          // ARCOUNT
    int off = 12;
    // Copy question section
    while (off < len && buf[off] != 0) off += buf[off] + 1;
    off += 5; // null byte + QTYPE(2) + QCLASS(2)
    if (off > len) return;
    memcpy(resp + 12, buf + 12, off - 12);
    int roff = off;
    // Answer: pointer to name in question
    resp[roff++] = 0xC0; resp[roff++] = 0x0C; // ptr to question name
    resp[roff++] = 0;  resp[roff++] = 1;  // TYPE A
    resp[roff++] = 0;  resp[roff++] = 1;  // CLASS IN
    resp[roff++] = 0;  resp[roff++] = 0;  resp[roff++] = 0;  resp[roff++] = 60; // TTL 60s
    resp[roff++] = 0;  resp[roff++] = 4;  // RDLENGTH
    resp[roff++] = 192; resp[roff++] = 168; resp[roff++] = 1; resp[roff++] = 1; // 192.168.1.1

    _dns.beginPacket(_dns.remoteIP(), _dns.remotePort());
    _dns.write(resp, roff);
    _dns.endPacket();
}

void WebServer::_handleClient(WiFiClient& c) {
    uint32_t timeout = millis() + 2000;
    while (!c.available() && millis() < timeout) delay(1);
    if (!c.available()) { c.stop(); return; }

    char method[8]  = {0};
    char path[128]  = {0};
    int  content_len = 0;
    char body[512]  = {0};

    // Read request line
    char line[256] = {0};
    if (!_parseLine(c, line, sizeof(line))) { c.stop(); return; }
    sscanf(line, "%7s %127s", method, path);

    // Read headers
    while (_parseLine(c, line, sizeof(line)) && strlen(line) > 0) {
        if (strncasecmp(line, "Content-Length:", 15) == 0)
            content_len = atoi(line + 15);
    }

    // Read body
    if (content_len > 0 && content_len < (int)sizeof(body) - 1) {
        int got = 0;
        timeout = millis() + 3000;
        while (got < content_len && millis() < timeout) {
            if (c.available()) body[got++] = c.read();
        }
        body[got] = 0;
    }

    // Route
    if (strcmp(path, "/events") == 0) {
        _serveSSE(c); return; // keep connection
    }
    if (strncmp(path, "/api/", 5) == 0) {
        if (!Api.handle(c, method, path, body, content_len)) _serve404(c);
        c.stop(); return;
    }
    // All other paths → main page (captive portal)
    _serveIndex(c);
    c.stop();
}

bool WebServer::_parseLine(WiFiClient& c, char* buf, int maxlen) {
    uint32_t t = millis() + 1000;
    int i = 0;
    while (millis() < t) {
        if (!c.available()) { delay(1); continue; }
        char ch = c.read();
        if (ch == '\r') continue;
        if (ch == '\n') { buf[i] = 0; return true; }
        if (i < maxlen - 1) buf[i++] = ch;
    }
    buf[i] = 0;
    return i > 0;
}

void WebServer::_serveIndex(WiFiClient& c) {
    c.printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
             "Content-Length: %d\r\nConnection: close\r\n\r\n",
             (int)strlen(INDEX_HTML));
    // Send in chunks to avoid buffer overflow
    const char* p = INDEX_HTML;
    int remaining = strlen(INDEX_HTML);
    while (remaining > 0) {
        int chunk = min(remaining, 512);
        c.write((const uint8_t*)p, chunk);
        p += chunk; remaining -= chunk;
    }
}

void WebServer::_serve404(WiFiClient& c) {
    c.print("HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found");
}

void WebServer::_serveSSE(WiFiClient& c) {
    c.print("HTTP/1.1 200 OK\r\n"
            "Content-Type: text/event-stream\r\n"
            "Cache-Control: no-cache\r\n"
            "Connection: keep-alive\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n");
    // Replace previous SSE client
    if (_sse && _sse.connected()) _sse.stop();
    _sse = c;
    Log.log(LOG_INFO, "SSE client connected");
}

void WebServer::_tickSSE() {
    if (!_sse || !_sse.connected()) return;
    uint32_t now = millis();
    if (now - _lastSSE < SSE_INTERVAL_MS) return;
    _lastSSE = now;

    const AttackStats& s = Attack.stats();
    char json[200];
    snprintf(json, sizeof(json),
        "{\"type\":\"stats\",\"deauths\":%lu,\"beacons\":%lu,"
        "\"auth\":%lu,\"assoc\":%lu,\"status\":\"%s\"}",
        s.deauths_sent, s.beacons_sent, s.auth_sent, s.assoc_sent,
        Attack.isRunning() ? "atk" : "idle");
    pushSSE(json);
}

void WebServer::pushSSE(const char* json) {
    if (!_sse || !_sse.connected()) return;
    _sse.printf("data: %s\n\n", json);
}
