#include "api.h"
#include "../wifi/scanner.h"
#include "../wifi/attacker.h"
#include "../wifi/sniffer.h"
#include "../system/nvs.h"
#include "../system/ota.h"
#include "../system/led.h"
#include "../log.h"
#include <WiFi.h>
#include <string.h>
#include <stdio.h>

ApiHandler Api;

static bool _startsWith(const char* s, const char* pre) {
    return strncmp(s, pre, strlen(pre)) == 0;
}

bool ApiHandler::handle(WiFiClient& client, const char* method, const char* path,
                         const char* body, int body_len) {
    // Strip query string for routing
    char clean[64]; strlcpy(clean, path, sizeof(clean));
    char* q = strchr(clean, '?'); if (q) *q = 0;
    const char* query = q ? (path + (q - clean) + 1) : "";

    if (strcmp(clean, "/api/scan") == 0) {
        if (strcmp(method,"POST")==0) _postScan(client);
        else _getScan(client);
        return true;
    }
    if (strcmp(clean, "/api/networks")==0)          { _getNetworks(client); return true; }
    if (strcmp(clean, "/api/attack/start")==0)       { _postAttackStart(client, body, body_len); return true; }
    if (strcmp(clean, "/api/attack/stop")==0)        { _postAttackStop(client); return true; }
    if (strcmp(clean, "/api/attack/stats")==0)       { _getAttackStats(client); return true; }
    if (strcmp(clean, "/api/sniff/start")==0)        { _postSniffStart(client); return true; }
    if (strcmp(clean, "/api/sniff/stop")==0)         { _postSniffStop(client); return true; }
    if (strcmp(clean, "/api/sniff/probes")==0)       { _getSniffProbes(client); return true; }
    if (strcmp(clean, "/api/sniff/probes/clear")==0) { _postSniffClearProbes(client); return true; }
    if (strcmp(clean, "/api/sniff/clients")==0)      { _getSniffClients(client, query); return true; }
    if (strcmp(clean, "/api/sniff/clients/clear")==0){ _postSniffClearClients(client); return true; }
    if (strcmp(clean, "/api/sniff/pmkids")==0)       { _getSniffPMKIDs(client); return true; }
    if (strcmp(clean, "/api/sniff/handshakes")==0)   { _getSniffHandshakes(client); return true; }
    if (strcmp(clean, "/api/beacon/start")==0)       { _postBeaconStart(client, body, body_len); return true; }
    if (strcmp(clean, "/api/beacon/stop")==0)        { _postBeaconStop(client); return true; }
    if (strcmp(clean, "/api/log")==0)                { _getLog(client); return true; }
    if (strcmp(clean, "/api/log/clear")==0)          { _postLogClear(client); return true; }
    if (strcmp(clean, "/api/settings")==0) {
        if (strcmp(method,"POST")==0) _postSettings(client, body, body_len);
        else _getSettings(client);
        return true;
    }
    if (strcmp(clean, "/api/settings/reset")==0) { _postSettingsReset(client); return true; }
    if (strcmp(clean, "/api/ota")==0)            { _postOta(client, body_len); return true; }
    return false;
}

void ApiHandler::_respond(WiFiClient& c, int code, const char* ct, const char* body) {
    const char* status = code==200?"OK":code==400?"Bad Request":"Internal Server Error";
    char hdr[192];
    snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nAccess-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\nConnection: close\r\n\r\n",
        code, status, ct, (int)strlen(body));
    c.print(hdr);
    c.print(body);
}
void ApiHandler::_respondJson(WiFiClient& c, const char* json) { _respond(c, 200, "application/json", json); }
void ApiHandler::_respondOk(WiFiClient& c)   { _respondJson(c, "{\"ok\":true}"); }
void ApiHandler::_respondErr(WiFiClient& c, const char* msg) {
    char buf[128]; snprintf(buf, sizeof(buf), "{\"ok\":false,\"error\":\"%s\"}", msg);
    _respond(c, 400, "application/json", buf);
}

// Escape JSON string: replace " \ \n \r with safe sequences
static void _jsonEsc(const char* src, char* dst, int maxlen) {
    int j = 0;
    for (int i = 0; src[i] && j < maxlen - 2; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"' || c == '\\') {
            if (j < maxlen - 3) dst[j++] = '\\';
        } else if (c == '\n') {
            if (j < maxlen - 3) { dst[j++] = '\\'; dst[j++] = 'n'; }
            continue;
        } else if (c == '\r') {
            continue;
        } else if (c < 0x20) {
            continue; // drop other control chars
        }
        dst[j++] = (char)c;
    }
    dst[j] = 0;
}

void ApiHandler::_bssidStr(const uint8_t* b, char* out) {
    snprintf(out, 18, "%02X:%02X:%02X:%02X:%02X:%02X", b[0],b[1],b[2],b[3],b[4],b[5]);
}
void ApiHandler::_hexStr(const uint8_t* b, int len, char* out) {
    for (int i = 0; i < len; i++) snprintf(out + i*2, 3, "%02x", b[i]);
    out[len*2] = 0;
}

// Minimal JSON key extraction
int ApiHandler::_jsonInt(const char* body, const char* key, int def) {
    if (!body) return def;
    char kk[32]; snprintf(kk, sizeof(kk), "\"%s\":", key);
    const char* p = strstr(body, kk);
    if (!p) return def;
    p += strlen(kk); while (*p==' ') p++;
    return atoi(p);
}
bool ApiHandler::_jsonStr(const char* body, const char* key, char* out, int maxlen) {
    if (!body) return false;
    char kk[32]; snprintf(kk, sizeof(kk), "\"%s\":", key);
    const char* p = strstr(body, kk);
    if (!p) return false;
    p += strlen(kk); while (*p==' ') p++;
    if (*p != '"') return false; p++;
    int i = 0;
    while (*p && *p != '"' && i < maxlen-1) out[i++] = *p++;
    out[i] = 0; return true;
}
bool ApiHandler::_jsonBool(const char* body, const char* key, bool def) {
    int v = _jsonInt(body, key, -1);
    if (v >= 0) return v != 0;
    char kk[32]; snprintf(kk, sizeof(kk), "\"%s\":", key);
    const char* p = strstr(body, kk);
    if (!p) return def;
    p += strlen(kk); while (*p==' ') p++;
    if (strncmp(p,"true",4)==0) return true;
    if (strncmp(p,"false",5)==0) return false;
    return def;
}

// --- Endpoints ---

void ApiHandler::_postScan(WiFiClient& c) {
    Scan.scan();
    _getNetworks(c);
}
void ApiHandler::_getScan(WiFiClient& c) { _getNetworks(c); }

void ApiHandler::_getNetworks(WiFiClient& c) {
    String json = "{\"networks\":[";
    char bssid[18];
    for (int i = 0; i < Scan.networkCount(); i++) {
        const Network* n = Scan.network(i);
        if (!n) continue;
        _bssidStr(n->bssid, bssid);
        if (i > 0) json += ',';
        char ssid_e[70]; _jsonEsc(n->ssid, ssid_e, sizeof(ssid_e));
        char buf[200];
        snprintf(buf, sizeof(buf),
            "{\"i\":%d,\"ssid\":\"%s\",\"bssid\":\"%s\","
            "\"rssi\":%d,\"ch\":%d,\"enc\":%d,"
            "\"freq\":%d,\"wps\":%s,\"pmf\":%s,\"hidden\":%s,\"clients\":%d}",
            i, ssid_e, bssid,
            n->rssi, n->channel, n->encryption,
            n->freq, n->wps?"true":"false", n->pmf?"true":"false",
            n->hidden?"true":"false", n->client_count);
        json += buf;
    }
    json += "],\"count\":";
    json += Scan.networkCount();
    json += ",\"ts\":";
    json += millis();
    json += "}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_postAttackStart(WiFiClient& c, const char* body, int len) {
    if (!body) { _respondErr(c, "no body"); return; }
    int mode   = _jsonInt(body, "mode", 1);
    int frames = _jsonInt(body, "frames", DEFAULT_DEAUTH_FRAMES);
    int delay  = _jsonInt(body, "delay", DEFAULT_SEND_DELAY);
    int reason = _jsonInt(body, "reason", DEFAULT_DEAUTH_REASON);

    Attack.setFrames(frames);
    Attack.setDelay(delay);
    Attack.setReason(reason);
    Attack.clearTargets();

    // Parse targets array: find all "bssid":"XX:XX..." patterns
    const char* p = body;
    char bssid_s[18], client_s[18];
    while ((p = strstr(p, "\"bssid\":\"")) != nullptr) {
        p += 9;
        char tmp[18]; int i = 0;
        while (*p && *p != '"' && i < 17) tmp[i++] = *p++;
        tmp[i] = 0;
        // Parse client if present
        const char* cp = strstr(p, "\"client\":\"");
        char ctmp[18] = {0};
        if (cp) { cp+=10; i=0; while(*cp&&*cp!='"'&&i<17) ctmp[i++]=*cp++; ctmp[i]=0; }

        uint8_t bssid_b[6]={0}, client_b[6]={0};
        sscanf(tmp, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &bssid_b[0],&bssid_b[1],&bssid_b[2],&bssid_b[3],&bssid_b[4],&bssid_b[5]);
        if (ctmp[0]) sscanf(ctmp, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &client_b[0],&client_b[1],&client_b[2],&client_b[3],&client_b[4],&client_b[5]);
        Attack.addTarget(bssid_b, ctmp[0] ? client_b : nullptr);
    }

    if (Attack.targetCount() == 0 && mode != 5) { // beacon spam needs no targets
        _respondErr(c, "no targets"); return;
    }

    Attack.startAttack((AttackMode)mode);
    Led.setState(LED_ATTACK);
    _respondOk(c);
}

void ApiHandler::_postAttackStop(WiFiClient& c) {
    Attack.stopAttack();
    Led.setState(LED_IDLE);
    _respondOk(c);
}

void ApiHandler::_getAttackStats(WiFiClient& c) {
    const AttackStats& s = Attack.stats();
    char buf[200];
    snprintf(buf, sizeof(buf),
        "{\"deauths\":%lu,\"beacons\":%lu,\"auth\":%lu,\"assoc\":%lu,\"status\":\"%s\"}",
        s.deauths_sent, s.beacons_sent, s.auth_sent, s.assoc_sent,
        Attack.isRunning() ? "atk" : "idle");
    _respondJson(c, buf);
}

void ApiHandler::_postSniffStart(WiFiClient& c) {
    Sniff.start(); Led.setState(LED_SCAN); _respondOk(c);
}
void ApiHandler::_postSniffStop(WiFiClient& c) {
    Sniff.stop(); Led.setState(LED_IDLE); _respondOk(c);
}

void ApiHandler::_getSniffProbes(WiFiClient& c) {
    String json = "{\"probes\":[";
    char mac[18];
    for (int i = 0; i < Sniff.probeCount(); i++) {
        const ProbeEntry* p = Sniff.probe(i);
        if (!p) continue;
        if (i > 0) json += ',';
        _bssidStr(p->mac, mac);
        char buf[160];
        char ssid_e[70]; _jsonEsc(p->ssid, ssid_e, sizeof(ssid_e));
        snprintf(buf, sizeof(buf),
            "{\"mac\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,"
            "\"age_first\":%lu,\"count\":%d}",
            mac, ssid_e, p->rssi,
            millis() - p->first_seen, p->count);
        json += buf;
    }
    json += "],\"count\":"; json += Sniff.probeCount(); json += "}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_getSniffClients(WiFiClient& c, const char* query) {
    // Filter by bssid if query has bssid=XX:XX:...
    uint8_t filter[6] = {0};
    bool hasFilter = false;
    if (query && strstr(query, "bssid=")) {
        const char* bp = strstr(query, "bssid=") + 6;
        sscanf(bp, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &filter[0],&filter[1],&filter[2],&filter[3],&filter[4],&filter[5]);
        hasFilter = true;
    }
    String json = "{\"clients\":[";
    char mac[18], ap[18]; bool first = true;
    for (int i = 0; i < Sniff.clientCount(); i++) {
        const ClientEntry* cl = Sniff.client(i);
        if (!cl) continue;
        if (hasFilter && memcmp(cl->ap_bssid, filter, 6) != 0) continue;
        if (!first) json += ','; first = false;
        _bssidStr(cl->client_mac, mac);
        _bssidStr(cl->ap_bssid, ap);
        char buf[120];
        snprintf(buf, sizeof(buf), "{\"mac\":\"%s\",\"ap\":\"%s\",\"age\":%lu}",
                 mac, ap, millis() - cl->last_seen);
        json += buf;
    }
    json += "]}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_getSniffPMKIDs(WiFiClient& c) {
    String json = "{\"pmkids\":[";
    char bssid[18], client[18], pmkid_hex[33];
    for (int i = 0; i < Sniff.pmkidCount(); i++) {
        const PMKIDEntry* p = Sniff.pmkid(i);
        if (!p) continue;
        if (i > 0) json += ',';
        _bssidStr(p->bssid, bssid);
        _bssidStr(p->client, client);
        _hexStr(p->pmkid, 16, pmkid_hex);
        char buf[200];
        char ssid_e[70]; _jsonEsc(p->ssid, ssid_e, sizeof(ssid_e));
        snprintf(buf, sizeof(buf),
            "{\"bssid\":\"%s\",\"ssid\":\"%s\",\"client\":\"%s\","
            "\"pmkid\":\"%s\",\"age\":%lu}",
            bssid, ssid_e, client, pmkid_hex, millis() - p->ts);
        json += buf;
    }
    json += "]}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_getSniffHandshakes(WiFiClient& c) {
    String json = "{\"handshakes\":[";
    char bssid[18], client[18], mic_hex[33];
    for (int i = 0; i < Sniff.handshakeCount(); i++) {
        const HandshakeEntry* h = Sniff.handshake(i);
        if (!h) continue;
        if (i > 0) json += ',';
        _bssidStr(h->bssid, bssid);
        _bssidStr(h->client, client);
        _hexStr(h->mic, 16, mic_hex);
        int msgs = __builtin_popcount(h->frames_mask);
        char buf[180];
        snprintf(buf, sizeof(buf),
            "{\"bssid\":\"%s\",\"client\":\"%s\",\"msgs\":%d,"
            "\"mic\":\"%s\",\"age\":%lu}",
            bssid, client, msgs, mic_hex, millis() - h->ts);
        json += buf;
    }
    json += "]}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_postSniffClearProbes(WiFiClient& c)  { Sniff.clearProbes();   _respondOk(c); }
void ApiHandler::_postSniffClearClients(WiFiClient& c) { Sniff.clearClients();  _respondOk(c); }

void ApiHandler::_postBeaconStart(WiFiClient& c, const char* body, int len) {
    if (!body) { _respondErr(c, "no body"); return; }
    int ch   = _jsonInt(body, "channel", 6);
    int intv = _jsonInt(body, "interval", 100);
    Attack.clearBeaconSSIDs();
    Attack.setBeaconChannel(ch);
    Attack.setBeaconInterval(intv);

    // Parse ssids array: extract all strings in "ssids":[...]
    const char* arr = strstr(body, "\"ssids\":");
    if (arr) {
        arr = strchr(arr, '['); if (!arr) { _respondErr(c, "no ssids"); return; }
        arr++;
        while (*arr) {
            while (*arr == ' ' || *arr == ',') arr++;
            if (*arr == ']') break;
            if (*arr == '"') {
                arr++;
                char ssid[33]; int i = 0;
                while (*arr && *arr != '"' && i < 32) ssid[i++] = *arr++;
                ssid[i] = 0; if (*arr=='"') arr++;
                Attack.addBeaconSSID(ssid);
            } else arr++;
        }
    }
    Attack.startAttack(ATK_BEACON_SPAM);
    Led.setState(LED_ATTACK);
    _respondOk(c);
}

void ApiHandler::_postBeaconStop(WiFiClient& c) {
    Attack.stopAttack(); Led.setState(LED_IDLE); _respondOk(c);
}

void ApiHandler::_getLog(WiFiClient& c) {
    static const char* lvlNames[] = {"info","warn","error","attack","sniff"};
    String json = "{\"entries\":[";
    int cnt = Log.count();
    const LogEntry* entries = Log.entries();
    bool first = true;
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        const LogEntry& e = entries[i];
        if (e.ts == 0 && e.msg[0] == 0) continue;
        if (!first) json += ','; first = false;
        char msg_e[100]; _jsonEsc(e.msg, msg_e, sizeof(msg_e));
        char buf[140];
        snprintf(buf, sizeof(buf), "{\"ts\":%lu,\"level\":\"%s\",\"msg\":\"%s\"}",
                 e.ts, lvlNames[(int)e.level < 4 ? (int)e.level : 4], msg_e);
        json += buf;
    }
    json += "]}";
    _respondJson(c, json.c_str());
}

void ApiHandler::_postLogClear(WiFiClient& c) { Log.clear(); _respondOk(c); }

void ApiHandler::_getSettings(WiFiClient& c) {
    Settings& s = Nvs.cfg();
    char buf[300];
    snprintf(buf, sizeof(buf),
        "{\"frames\":%d,\"delay\":%d,\"ap_ssid\":\"%s\",\"ap_pass\":\"%s\","
        "\"ap_ch\":%d,\"auto_scan\":%d,\"auto_atk\":%d,\"led\":%d,\"version\":\"%s\"}",
        s.deauth_frames, s.send_delay_ms, s.ap_ssid, "****",
        s.ap_channel, s.auto_scan, s.auto_attack, s.led_enabled, FW_VERSION);
    _respondJson(c, buf);
}

void ApiHandler::_postSettings(WiFiClient& c, const char* body, int len) {
    if (!body) { _respondErr(c, "no body"); return; }
    Settings& s = Nvs.cfg();
    int v;
    char tmp[64];
    v = _jsonInt(body, "frames", -1);  if (v > 0) s.deauth_frames = v;
    v = _jsonInt(body, "delay", -1);   if (v >= 0) s.send_delay_ms = v;
    v = _jsonInt(body, "ap_ch", -1);   if (v > 0 && v <= 165) s.ap_channel = v;
    v = _jsonInt(body, "auto_scan", -1); if (v >= 0) s.auto_scan = v;
    v = _jsonInt(body, "auto_atk", -1);  if (v >= 0) s.auto_attack = v;
    v = _jsonInt(body, "led", -1);       if (v >= 0) { s.led_enabled = v; Led.setEnabled(v); }
    if (_jsonStr(body, "ap_ssid", tmp, sizeof(tmp))) strlcpy(s.ap_ssid, tmp, sizeof(s.ap_ssid));
    if (_jsonStr(body, "ap_pass", tmp, sizeof(tmp))) {
        if (strlen(tmp) >= 8)
            strlcpy(s.ap_pass, tmp, sizeof(s.ap_pass));
        else { _respondErr(c, "password min 8 chars"); return; }
    }
    Nvs.save();
    _respondOk(c);
}

void ApiHandler::_postSettingsReset(WiFiClient& c) { Nvs.reset(); _respondOk(c); }

void ApiHandler::_postOta(WiFiClient& c, int content_len) {
    bool ok = Ota.handleUpload(c, content_len);
    if (ok) _respond(c, 200, "text/plain", "OK - rebooting");
    else _respondErr(c, "OTA failed");
}
