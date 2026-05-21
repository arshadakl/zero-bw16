#include "sniffer.h"
#include "attacker.h"
#include "scanner.h"
#include "../log.h"

extern "C" {
    int wifi_set_promisc(int rcv_type, void (*cb)(unsigned char*, unsigned int, void*), unsigned char en);
}

// 802.11 FC constants
#define FC_PROBE_REQ_TYPE  0x40
#define FC_ASSOC_REQ_TYPE  0x00
#define FC_ASSOC_RSP_TYPE  0x10
#define FC_DATA_TYPE       0x08

// EAPOL LLC header (0xAA 0xAA 0x03 0x00 0x00 0x00 0x88 0x8E)
static const uint8_t LLC_EAPOL[8] = {0xAA,0xAA,0x03,0x00,0x00,0x00,0x88,0x8E};

Sniffer Sniff;

void Sniffer::init() {
    _running = false;
    _probeCount = 0;
    _clientCount = 0;
    _pmkidCount = 0;
    _hsCount = 0;
}

void Sniffer::start() {
    wifi_set_promisc(0, _onFrame, 1); // 0 = all packets
    _running = true;
    Log.log(LOG_SNIFF, "Sniffer started");
}

void Sniffer::stop() {
    wifi_set_promisc(0, nullptr, 0);
    _running = false;
    Log.log(LOG_SNIFF, "Sniffer stopped. P:%d C:%d PMKID:%d HS:%d",
            _probeCount, _clientCount, _pmkidCount, _hsCount);
}

bool Sniffer::isRunning() const { return _running; }
int Sniffer::probeCount() const { return _probeCount; }
const ProbeEntry* Sniffer::probe(int i) const {
    if (i < 0 || i >= _probeCount) return nullptr;
    return &_probes[i];
}
int Sniffer::clientCount() const { return _clientCount; }
const ClientEntry* Sniffer::client(int i) const {
    if (i < 0 || i >= _clientCount) return nullptr;
    return &_clients[i];
}
int Sniffer::pmkidCount() const { return _pmkidCount; }
const PMKIDEntry* Sniffer::pmkid(int i) const {
    if (i < 0 || i >= _pmkidCount) return nullptr;
    return &_pmkids[i];
}
int Sniffer::handshakeCount() const { return _hsCount; }
const HandshakeEntry* Sniffer::handshake(int i) const {
    if (i < 0 || i >= _hsCount) return nullptr;
    return &_handshakes[i];
}

void Sniffer::clearProbes()    { _probeCount = 0; _changes++; }
void Sniffer::clearClients()   { _clientCount = 0; _changes++; }
void Sniffer::clearCaptures()  { _pmkidCount = 0; _hsCount = 0; _changes++; }
uint32_t Sniffer::changeCounter() const { return _changes; }

void Sniffer::_onFrame(unsigned char* buf, unsigned int len, void* userdata) {
    if (len < 24) return;
    uint8_t fc0 = buf[0];
    uint8_t fc1 = buf[1];
    int8_t rssi = 0;
    // Realtek appends RSSI at end of promisc buffer in some driver versions
    if (len > 24) rssi = (int8_t)buf[len - 1];

    uint8_t type    = fc0 & 0x0C;
    uint8_t subtype = fc0 & 0xF0;

    if (type == 0x00) { // management
        if (subtype == FC_PROBE_REQ_TYPE)
            Sniff._handleProbe(buf, len, rssi);
        else if (subtype == FC_ASSOC_REQ_TYPE || subtype == FC_ASSOC_RSP_TYPE)
            Sniff._handleAssoc(buf, len);
    } else if (type == FC_DATA_TYPE) {
        // Check for EAPOL
        if (len > 32) Sniff._handleEAPOL(buf, len);
    }
}

void Sniffer::_handleProbe(const uint8_t* frame, int len, int8_t rssi) {
    if (len < 28) return;
    const uint8_t* sa = frame + 10; // SA in header

    // Parse SSID IE at offset 24
    const uint8_t* ie = frame + 24;
    int remaining = len - 24;
    char ssid[33] = {0};
    if (remaining >= 2 && ie[0] == 0) {
        uint8_t ssid_len = min((int)ie[1], 32);
        memcpy(ssid, ie + 2, ssid_len);
        ssid[ssid_len] = 0;
    }

    // Karma: if attack running, respond
    extern Attacker Attack;
    if (Attack.isRunning() && Attack.currentMode() == ATK_KARMA) {
        Attack.karmaRespond(sa, ssid);
    }

    // Store probe
    for (int i = 0; i < _probeCount; i++) {
        if (memcmp(_probes[i].mac, sa, 6) == 0 &&
            strcmp(_probes[i].ssid, ssid) == 0) {
            _probes[i].rssi = rssi;
            _probes[i].last_seen = millis();
            _probes[i].count++;
            _changes++;
            return;
        }
    }
    if (_probeCount >= MAX_PROBE_ENTRIES) return;
    ProbeEntry& p = _probes[_probeCount++];
    memcpy(p.mac, sa, 6);
    strlcpy(p.ssid, ssid, sizeof(p.ssid));
    p.rssi = rssi;
    p.first_seen = p.last_seen = millis();
    p.count = 1;
    _changes++;
}

void Sniffer::_handleAssoc(const uint8_t* frame, int len) {
    if (len < 24) return;
    const uint8_t* da    = frame + 4;   // destination (AP BSSID)
    const uint8_t* sa    = frame + 10;  // source (client)
    const uint8_t* bssid = frame + 16;

    // Update scanner client count
    Scan.updateClientCount(bssid, 0); // refresh reference

    // Store or update client entry
    for (int i = 0; i < _clientCount; i++) {
        if (memcmp(_clients[i].client_mac, sa, 6) == 0 &&
            memcmp(_clients[i].ap_bssid, bssid, 6) == 0) {
            _clients[i].last_seen = millis();
            return;
        }
    }
    if (_clientCount >= MAX_CLIENT_ENTRIES) return;
    ClientEntry& c = _clients[_clientCount++];
    memcpy(c.client_mac, sa, 6);
    memcpy(c.ap_bssid, bssid, 6);
    c.last_seen = millis();
    Scan.updateClientCount(bssid, 1);
    _changes++;
}

void Sniffer::_handleEAPOL(const uint8_t* frame, int len) {
    // 802.11 header = 24 bytes, then LLC (8 bytes for EAPOL)
    if (len < 36) return;
    const uint8_t* da    = frame + 4;
    const uint8_t* sa    = frame + 10;
    const uint8_t* bssid = frame + 16;
    const uint8_t* llc   = frame + 24;

    if (memcmp(llc, LLC_EAPOL, 8) != 0) return;
    // EAPOL frame starts at offset 32
    const uint8_t* eapol = frame + 32;
    int eapol_len = len - 32;
    if (eapol_len < 4) return;

    // uint8_t eapol_type = eapol[1]; // Key = 3
    if (eapol[1] != 3) return; // not EAPOL-Key
    if (eapol_len < 99) return;

    uint8_t key_info_hi = eapol[5]; // KeyInfo byte 0
    uint8_t msg_num = 0;
    bool ack = (key_info_hi & 0x08) != 0;  // ACK bit
    bool mic = (key_info_hi & 0x01) != 0;  // MIC bit

    if (ack && !mic) msg_num = 1;       // msg 1/3
    else if (!ack && mic) msg_num = 2;  // msg 2/4

    // EAPOL-Key body starts at eapol[4]; nonce field at body+13 = eapol[17]
    // SNonce (msg2) occupies the same nonce field position as ANonce (msg1)
    const uint8_t* nonce = eapol + 17;

    // Key Data length at eapol[97..98], key data at eapol[99]
    uint16_t kd_len = (eapol[97] << 8) | eapol[98];
    const uint8_t* kd = eapol + 99;

    if (msg_num == 1 && kd_len >= 22) {
        // PMKID in RSNE IE of msg1 key data
        _extractPMKID(bssid, sa, kd, kd_len);
    }

    // Track 4-way handshake
    bool found = false;
    for (int i = 0; i < _hsCount; i++) {
        HandshakeEntry& h = _handshakes[i];
        if (memcmp(h.bssid, bssid, 6) == 0 &&
            memcmp(h.client, sa, 6) == 0) {
            h.frames_mask |= (1 << (msg_num - 1));
            if (msg_num == 1) memcpy(h.anonce, nonce, 32);
            if (msg_num == 2) {
                memcpy(h.snonce, nonce, 32);
                if (eapol_len >= 93) memcpy(h.mic, eapol + 81, 16);
            }
            h.ts = millis();
            found = true;
            _changes++;
            break;
        }
    }
    if (!found && _hsCount < MAX_PMKID_ENTRIES) {
        HandshakeEntry& h = _handshakes[_hsCount++];
        memcpy(h.bssid, bssid, 6);
        memcpy(h.client, sa, 6);
        h.frames_mask = (1 << (msg_num - 1));
        memset(h.anonce, 0, 32); memset(h.snonce, 0, 32); memset(h.mic, 0, 16);
        if (msg_num == 1) memcpy(h.anonce, nonce, 32);
        if (msg_num == 2) {
            memcpy(h.snonce, nonce, 32);
            if (eapol_len >= 93) memcpy(h.mic, eapol + 81, 16);
        }
        h.ts = millis();
        _changes++;
        Log.log(LOG_SNIFF, "HS captured BSSID %02X:%02X:%02X...",
                bssid[0], bssid[1], bssid[2]);
    }
}

void Sniffer::_extractPMKID(const uint8_t* bssid, const uint8_t* client,
                              const uint8_t* kd, int kd_len) {
    int off = 0;
    while (off + 2 <= kd_len) {
        uint8_t id  = kd[off];
        uint8_t eln = kd[off + 1];
        if (off + 2 + (int)eln > kd_len) break;
        if (id == 0x30) {
            // Parse RSNE dynamically to locate PMKID at correct offset
            // RSNE body: Version(2) + GroupCipher(4) + PwCount(2) + PwSuites(4n)
            //          + AKMCount(2) + AKMSuites(4n) + Capabilities(2)
            //          + PMKIDCount(2) + PMKIDList(16n)
            const uint8_t* rsne = kd + off + 2;
            int roff = 2; // skip Version
            if (roff + 4 > eln) goto next_ie;
            roff += 4; // Group Cipher Suite
            if (roff + 2 > eln) goto next_ie;
            { uint16_t pw = rsne[roff] | (uint16_t)(rsne[roff+1] << 8);
              roff += 2 + 4 * pw; } // Pairwise count + suites
            if (roff + 2 > eln) goto next_ie;
            { uint16_t akm = rsne[roff] | (uint16_t)(rsne[roff+1] << 8);
              roff += 2 + 4 * akm; } // AKM count + suites
            roff += 2; // RSN Capabilities
            if (roff + 2 > eln) goto next_ie;
            { uint16_t pmkid_cnt = rsne[roff] | (uint16_t)(rsne[roff+1] << 8);
              roff += 2;
              if (pmkid_cnt > 0 && roff + 16 <= eln) {
                  const uint8_t* pmkid_ptr = rsne + roff;
                  for (int i = 0; i < _pmkidCount; i++) {
                      if (memcmp(_pmkids[i].bssid, bssid, 6) == 0 &&
                          memcmp(_pmkids[i].client, client, 6) == 0) return;
                  }
                  if (_pmkidCount >= MAX_PMKID_ENTRIES) return;
                  PMKIDEntry& p = _pmkids[_pmkidCount++];
                  memcpy(p.bssid, bssid, 6);
                  memcpy(p.client, client, 6);
                  memcpy(p.pmkid, pmkid_ptr, 16);
                  p.ts = millis();
                  const Network* net = Scan.networkByBSSID(bssid);
                  strlcpy(p.ssid, net ? net->ssid : "?", sizeof(p.ssid));
                  _changes++;
                  Log.log(LOG_SNIFF, "PMKID captured %02X:%02X:%02X...",
                          bssid[0], bssid[1], bssid[2]);
                  return;
              }
            }
        }
        next_ie:
        off += 2 + eln;
    }
}
