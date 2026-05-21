#include "framebuilder.h"

static const uint8_t BCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t RATES[]  = {0x82,0x84,0x8B,0x96,0x0C,0x12,0x18,0x24,0x30,0x48,0x60,0x6C};

uint16_t FrameBuilder::_seqNum = 0;

void FrameBuilder::randomMAC(uint8_t* mac) {
    for (int i = 0; i < 6; i++) mac[i] = (uint8_t)random(0, 256);
    mac[0] = (mac[0] & 0xFE) | 0x02; // unicast + locally administered
}

int FrameBuilder::_addIE(uint8_t* buf, uint8_t id, const uint8_t* data, uint8_t len) {
    buf[0] = id;
    buf[1] = len;
    if (len > 0 && data) memcpy(buf + 2, data, len);
    return 2 + len;
}

static void _fillHeader(Dot11Header* hdr, uint8_t fc0, const uint8_t* da,
                         const uint8_t* sa, const uint8_t* bssid) {
    hdr->fc[0] = fc0; hdr->fc[1] = 0x00;
    hdr->duration = 0x013A;
    memcpy(hdr->da, da, 6);
    memcpy(hdr->sa, sa, 6);
    memcpy(hdr->bssid, bssid, 6);
    hdr->seq = (FrameBuilder::_seqNum++ & 0xFFF) << 4;
}

// --- Deauth ---
int FrameBuilder::buildDeauth(uint8_t* buf, const uint8_t* bssid,
                               const uint8_t* da, uint16_t reason) {
    DeauthFrame* f = (DeauthFrame*)buf;
    _fillHeader(&f->hdr, FC_DEAUTH, da, bssid, bssid);
    f->reason = reason;
    return sizeof(DeauthFrame);
}

// --- Auth request (open, seq=1) ---
int FrameBuilder::buildAuth(uint8_t* buf, const uint8_t* bssid,
                             const uint8_t* sa, uint16_t alg) {
    AuthFrame* f = (AuthFrame*)buf;
    _fillHeader(&f->hdr, FC_AUTH, bssid, sa, bssid);
    f->alg    = alg;
    f->seq    = 1;
    f->status = 0;
    return sizeof(AuthFrame);
}

// --- Association request ---
int FrameBuilder::buildAssocReq(uint8_t* buf, const uint8_t* bssid,
                                  const uint8_t* sa, const char* ssid) {
    int off = 0;
    AssocReqFrame* f = (AssocReqFrame*)buf;
    _fillHeader(&f->hdr, FC_ASSOC_REQ, bssid, sa, bssid);
    f->capability     = 0x0431;
    f->listen_interval = 0x000A;
    off = sizeof(AssocReqFrame);
    uint8_t ssidlen = ssid ? strlen(ssid) : 0;
    off += _addIE(buf + off, 0, (const uint8_t*)ssid, ssidlen);
    off += _addIE(buf + off, 1, RATES, sizeof(RATES));
    return off;
}

// --- Beacon ---
int FrameBuilder::buildBeacon(uint8_t* buf, const uint8_t* bssid,
                               const char* ssid, uint8_t channel,
                               uint16_t interval) {
    int off = 0;
    Dot11Header* hdr = (Dot11Header*)buf;
    _fillHeader(hdr, FC_BEACON, BCAST, bssid, bssid);
    off = sizeof(Dot11Header);

    BeaconFixed* fix = (BeaconFixed*)(buf + off);
    fix->timestamp  = millis();
    fix->interval   = interval;
    fix->capability = 0x0421; // ESS + short preamble
    off += sizeof(BeaconFixed);

    uint8_t ssidlen = ssid ? min((int)strlen(ssid), 32) : 0;
    off += _addIE(buf + off, 0, (const uint8_t*)ssid, ssidlen);
    off += _addIE(buf + off, 1, RATES, 8);
    off += _addIE(buf + off, 3, &channel, 1);
    return off;
}

// --- Probe request ---
int FrameBuilder::buildProbeReq(uint8_t* buf, const uint8_t* sa, const char* ssid) {
    int off = 0;
    Dot11Header* hdr = (Dot11Header*)buf;
    _fillHeader(hdr, FC_PROBE_REQ, BCAST, sa, BCAST);
    off = sizeof(Dot11Header);
    uint8_t ssidlen = (ssid && *ssid) ? strlen(ssid) : 0;
    off += _addIE(buf + off, 0, (const uint8_t*)ssid, ssidlen);
    off += _addIE(buf + off, 1, RATES, 8);
    return off;
}
