#pragma once
#include <Arduino.h>
#include "../config.h"

struct ProbeEntry {
    uint8_t mac[6];
    char ssid[33];
    int8_t rssi;
    uint32_t first_seen;
    uint32_t last_seen;
    uint16_t count;
};

struct ClientEntry {
    uint8_t client_mac[6];
    uint8_t ap_bssid[6];
    uint32_t last_seen;
};

struct PMKIDEntry {
    uint8_t bssid[6];
    uint8_t client[6];
    uint8_t pmkid[16];
    uint32_t ts;
    char ssid[33];
};

struct HandshakeEntry {
    uint8_t bssid[6];
    uint8_t client[6];
    uint8_t frames_mask;  // bits 0-3 = which of 4 messages captured
    uint8_t anonce[32];
    uint8_t snonce[32];
    uint8_t mic[16];
    uint32_t ts;
};

class Sniffer {
public:
    void init();
    void start();
    void stop();
    bool isRunning() const;

    // Probe requests
    int probeCount() const;
    const ProbeEntry* probe(int i) const;

    // Client map
    int clientCount() const;
    const ClientEntry* client(int i) const;

    // PMKID
    int pmkidCount() const;
    const PMKIDEntry* pmkid(int i) const;

    // Handshakes
    int handshakeCount() const;
    const HandshakeEntry* handshake(int i) const;

    void clearProbes();
    void clearClients();
    void clearCaptures();
    uint32_t changeCounter() const;

    // Called by promisc callback (static bridge)
    static void _onFrame(unsigned char* buf, unsigned int len, void* userdata);

private:
    bool _running = false;
    uint32_t _changes = 0;

    ProbeEntry _probes[MAX_PROBE_ENTRIES];
    int _probeCount = 0;

    ClientEntry _clients[MAX_CLIENT_ENTRIES];
    int _clientCount = 0;

    PMKIDEntry _pmkids[MAX_PMKID_ENTRIES];
    int _pmkidCount = 0;

    HandshakeEntry _handshakes[MAX_PMKID_ENTRIES];
    int _hsCount = 0;

    void _handleProbe(const uint8_t* frame, int len, int8_t rssi);
    void _handleAssoc(const uint8_t* frame, int len);
    void _handleEAPOL(const uint8_t* frame, int len);
    void _extractPMKID(const uint8_t* bssid, const uint8_t* client,
                        const uint8_t* key_data, int kd_len);
};

extern Sniffer Sniff;
