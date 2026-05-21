# Changelog

All notable changes to zero-bw16 are documented here.

## [1.0.0] - 2026-05-21

### Added
- Multi-mode attack engine: deauth, auth flood, assoc flood, beacon spam, evil twin, karma
- Passive WiFi sniffer: probe requests, client-AP map, PMKID capture, handshake capture
- Single-page web UI with dark/light mode and 5 feature tabs
- Real-time SSE event push (no page reload required)
- REST JSON API for all features
- DNS captive portal redirecting all queries to 192.168.1.1
- NVS settings persistence via EEPROM emulation
- OTA firmware update via web UI drag-and-drop
- Auto-scan and auto-attack flags on boot
- Hardware button: short press = scan, long press = toggle attack
- LED state machine: idle blink / fast scan / solid attack / double-blink error
- 802.11 raw management frame builder (deauth/beacon/auth/assoc/probe)
- PMKID extraction from EAPOL message 1
- WPA2 4-way handshake frame capture
