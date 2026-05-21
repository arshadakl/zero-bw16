#pragma once
#include <Arduino.h>

// 802.11 Frame Control subtypes
#define FC_ASSOC_REQ   0x00
#define FC_PROBE_REQ   0x40
#define FC_BEACON      0x80
#define FC_DISASSOC    0xA0
#define FC_AUTH        0xB0
#define FC_DEAUTH      0xC0

#pragma pack(push, 1)
struct Dot11Header {
    uint8_t  fc[2];
    uint16_t duration;
    uint8_t  da[6];
    uint8_t  sa[6];
    uint8_t  bssid[6];
    uint16_t seq;
};

struct DeauthFrame {
    Dot11Header hdr;
    uint16_t reason;
};

struct AuthFrame {
    Dot11Header hdr;
    uint16_t alg;
    uint16_t seq;
    uint16_t status;
};

struct AssocReqFrame {
    Dot11Header hdr;
    uint16_t capability;
    uint16_t listen_interval;
    // followed by IEs
};

struct BeaconFixed {
    uint64_t timestamp;
    uint16_t interval;
    uint16_t capability;
};
#pragma pack(pop)

class FrameBuilder {
public:
    // Returns frame length written into buf
    static int buildDeauth(uint8_t* buf, const uint8_t* bssid,
                           const uint8_t* da, uint16_t reason);

    static int buildAuth(uint8_t* buf, const uint8_t* bssid,
                         const uint8_t* sa, uint16_t alg = 0);

    static int buildAssocReq(uint8_t* buf, const uint8_t* bssid,
                              const uint8_t* sa, const char* ssid);

    static int buildBeacon(uint8_t* buf, const uint8_t* bssid,
                           const char* ssid, uint8_t channel,
                           uint16_t interval = 100);

    static int buildProbeReq(uint8_t* buf, const uint8_t* sa,
                              const char* ssid = nullptr);

    static void randomMAC(uint8_t* mac);

private:
    static int _addIE(uint8_t* buf, uint8_t id, const uint8_t* data, uint8_t len);
};
