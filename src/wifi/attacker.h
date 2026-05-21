#pragma once
#include <Arduino.h>
#include "../config.h"

enum AttackMode {
    ATK_NONE,
    ATK_DEAUTH,       // standard deauth per-AP
    ATK_DEAUTH_BCAST, // broadcast deauth (disconnect all clients)
    ATK_AUTH_FLOOD,   // flood AP with auth requests (spoofed MACs)
    ATK_ASSOC_FLOOD,  // flood AP with assoc requests
    ATK_BEACON_SPAM,  // flood area with fake beacons
    ATK_EVIL_TWIN,    // clone AP SSID/channel + start rogue AP
    ATK_KARMA,        // respond to all probe requests
};

struct AttackTarget {
    uint8_t bssid[6];
    uint8_t client[6]; // all-zero = broadcast target
    bool    active;
};

struct AttackStats {
    uint32_t deauths_sent;
    uint32_t beacons_sent;
    uint32_t auth_sent;
    uint32_t assoc_sent;
    uint32_t probes_sent;
    uint32_t start_time;
};

class Attacker {
public:
    void init();
    void setFrames(uint8_t frames);
    void setDelay(uint8_t delay_ms);
    void setReason(uint16_t reason);

    // Deauth
    void addTarget(const uint8_t* bssid, const uint8_t* client = nullptr);
    void clearTargets();
    int  targetCount() const;

    // Start/stop
    void startAttack(AttackMode mode);
    void stopAttack();
    bool isRunning() const;
    AttackMode currentMode() const;

    // Must be called once with the AP's operating channel so the attacker
    // can restore the radio back to AP channel after each injection burst.
    void setHomeChannel(uint8_t ch);

    // Beacon spam
    void addBeaconSSID(const char* ssid);
    void clearBeaconSSIDs();
    int  beaconSSIDCount() const;
    void setBeaconChannel(uint8_t ch);
    void setBeaconInterval(uint16_t ms);

    // Evil twin
    void setEvilTwinTarget(const uint8_t* bssid, const char* ssid, uint8_t channel);

    // Tick — call from loop()
    void tick();

    const AttackStats& stats() const;

    // Karma: called by sniffer when probe request received
    void karmaRespond(const uint8_t* da, const char* ssid);

private:
    AttackMode _mode = ATK_NONE;
    bool _running = false;
    uint8_t _frames = DEFAULT_DEAUTH_FRAMES;
    uint8_t _delay_ms = DEFAULT_SEND_DELAY;
    uint16_t _reason = DEFAULT_DEAUTH_REASON;

    AttackTarget _targets[MAX_NETWORKS];
    int _targetCount = 0;

    // Beacon spam
    char _beaconSSIDs[MAX_BEACON_SSIDS][33];
    int _beaconCount = 0;
    uint8_t _beaconChannel = 6;
    uint16_t _beaconInterval = 100;
    int _beaconIdx = 0;
    uint32_t _lastBeacon = 0;

    // Evil twin
    char _etSSID[33];
    uint8_t _etBSSID[6];
    uint8_t _etChannel = 1;
    bool _etActive = false;

    uint8_t _homeChannel = 6; // AP operating channel; restored after each TX burst

    AttackStats _stats;
    uint32_t _lastTx = 0;
    int _targetIdx = 0;

    void _tickDeauth();
    void _tickAuthFlood();
    void _tickAssocFlood();
    void _tickBeaconSpam();
    void _tickKarma();
    bool _txFrame(uint8_t* buf, int len);
};

extern Attacker Attack;
