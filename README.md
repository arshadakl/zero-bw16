<div align="center">
  <img src="https://raw.githubusercontent.com/arshadakl/assets/refs/heads/main/Zero-BW16-Security-Firmware.png" alt="Zero-BW16 Security Firmware" width="100%">
</div>

<div align="center">

![Platform](https://img.shields.io/badge/Platform-RTL8720DN%20%7C%20BW16-blue)
![Framework](https://img.shields.io/badge/Framework-Arduino-teal)
![Build](https://img.shields.io/badge/Build-PlatformIO-orange)
![License](https://img.shields.io/badge/License-GPL--3.0-red)
![Band](https://img.shields.io/badge/WiFi-2.4GHz%20%2B%205GHz-brightgreen)

</div>

---

> **Legal Notice:** This firmware is intended exclusively for **authorized penetration testing**, **CTF competitions**, **WiFi security research**, and **educational purposes**. Use only on networks and devices you own or have explicit written permission to test. Unauthorized use is illegal and unethical. The author assumes no liability for misuse.

---

## Overview

**Zero-BW16** is an open-source WiFi security auditing firmware built from the ground up for the **AI-Thinker BW16 (RTL8720DN)** dual-band module. It runs entirely on the BW16 hardware with no external peripherals, combining active attack capabilities, passive intelligence gathering, and a real-time web dashboard — all accessible over a local access point.

The firmware targets security researchers and penetration testers who need a compact, standalone, low-cost WiFi auditing tool.

---

## Features

### Active Auditing

| Feature | Description |
|---|---|
| **Deauthentication** | Directed or broadcast deauth frames targeting selected APs or clients |
| **Auth Flooding** | Flood a target AP with spoofed 802.11 authentication requests |
| **Association Flooding** | Flood a target AP with spoofed association requests |
| **Beacon Spam** | Broadcast custom or randomized fake SSIDs to pollute scan results |
| **Evil Twin AP** | Clone a target AP's SSID and channel to serve as a rogue access point |
| **Karma Attack** | Respond to all client probe requests with a matching fake SSID |
| **Broadcast Deauth** | Send deauth frames to FF:FF:FF:FF:FF:FF — disconnects all clients simultaneously |
| **Bidirectional Deauth** | Inject deauth frames in both AP→Client and Client→AP directions |

### Passive Intelligence

| Feature | Description |
|---|---|
| **Probe Request Sniffing** | Capture nearby device probe requests including source MAC and probed SSID |
| **Client-to-AP Mapping** | Build a real-time map of associated clients per access point |
| **WPS Detection** | Parse beacon IEs to identify WPS-enabled access points |
| **PMF Detection** | Parse RSN IE capability bits to flag Management Frame Protection status |
| **PMKID Extraction** | Extract PMKID from EAPOL Message 1 key data for offline cracking |
| **WPA2 Handshake Capture** | Capture all 4 frames of the EAPOL 4-way handshake |
| **Hidden SSID Tracking** | Log access points broadcasting empty SSIDs |
| **RSSI History** | Rolling 10-sample RSSI log per BSSID with trend tracking |

### Web Interface

| Feature | Description |
|---|---|
| **Local Dashboard** | Single-page web app served at `192.168.1.1` — no internet required |
| **Multi-Tab Navigation** | Attack / Sniff / Beacon Spam / Log / Settings |
| **Real-Time Updates** | Server-Sent Events (SSE) push updates without page refresh |
| **Dark / Light Theme** | CSS-variable theming, preference stored in `localStorage` |
| **OTA Firmware Update** | Drag-and-drop `.bin` upload at `/ota` — writes to second partition and reboots |
| **Attack Statistics** | Live counters for deauths sent, beacons injected, auth frames, probes captured |

### System

| Feature | Description |
|---|---|
| **Dual-Band Support** | Full 2.4 GHz (ch 1–13) and 5 GHz (UNII-1/2/3, ch 36–165) |
| **NVS Persistence** | Settings survive reboot via EEPROM-backed storage |
| **Auto-Scan on Boot** | Optional flag to run a network scan immediately at startup |
| **Auto-Attack on Boot** | Optional flag to resume last attack profile on power-up |
| **Hardware Button** | Single GPIO button — short press: scan, long press: toggle attack |
| **LED State Machine** | Idle=slow blink, Scan=fast blink, Attack=solid, Error=double blink |
| **DNS Captive Portal** | Embedded DNS server resolves all queries to `192.168.1.1` |
| **Circular Attack Log** | In-memory log buffer (200 entries) with timestamp and level |

---

## Hardware

| Spec | Value |
|---|---|
| Module | AI-Thinker BW16 (RTL8720DN) |
| SoC | Realtek RTL8720DN (KM0 + KM4 dual-core ARM) |
| WiFi | 802.11 a/b/g/n/ac — 2.4 GHz + 5 GHz |
| Flash | 2 MB |
| SRAM | 256 KB + PSRAM |
| LED Pin | PA27 (active low) |
| Button Pin | PA12 (optional, pulled high) |
| Power | USB Type-C, 3.3 V |

No external hardware is required. Everything runs on the BW16 module alone.

---

## Project Structure

```
src/
├── main.cpp                  — setup() and loop(), boot logic, SSE push
├── config.h                  — compile-time defaults, limits, 5GHz channel table
├── log.cpp / log.h           — circular buffer logger, 5 log levels
├── wifi/
│   ├── scanner.cpp/h         — dual-band scan, RSSI history, client count tracking
│   ├── attacker.cpp/h        — all attack modes, channel switching, frame dispatch
│   ├── sniffer.cpp/h         — promiscuous mode, EAPOL parsing, PMKID/handshake capture
│   └── framebuilder.cpp/h    — raw 802.11 frame construction (deauth/auth/assoc/beacon)
├── web/
│   ├── webserver.cpp/h       — HTTP + SSE server, DNS captive portal
│   ├── pages.h               — full HTML/CSS/JS UI embedded as string literal
│   └── api.cpp/h             — 20+ REST JSON endpoints
└── system/
    ├── nvs.cpp/h             — EEPROM-backed settings with magic validation
    ├── led.cpp/h             — LED state machine
    ├── button.cpp/h          — GPIO debounce, short/long press events
    └── ota.cpp/h             — OTA update via Realtek ota_update_* API
```

---

## Build & Flash

### Requirements

- [PlatformIO](https://platformio.org/) IDE or CLI
- `realtek-ambz2` platform installed
- USB Type-C cable (for initial flash only)

### Build

```bash
pio run
```

Output binary: `.pio/build/bw16/firmware.bin`

### Entering Download Mode

Required before any UART flash:

1. Hold the **BOOT** button
2. Press and release **RESET**
3. Release **BOOT**
4. The module is now in download mode — proceed with flashing

---

### Flash Method 1 — PlatformIO (Recommended)

Cross-platform, no extra tools needed.

```bash
pio run -t upload
```

PlatformIO auto-detects the COM port and flashes at 1.5 Mbps.

---

### Flash Method 2 — Realtek AmebaD ImageTool (Official)

Official Realtek GUI flasher for Windows. Distributed with the [AmebaD SDK](https://github.com/ambiot/ambd_sdk) under `tools/`.

Steps:
1. Open `AmebaD_ImageTool.exe`
2. Select COM port
3. Load `.pio/build/bw16/firmware.bin`
4. Click **Download**

---

### Flash Method 3 — OTA (Subsequent Updates)

After the initial UART flash, all future firmware updates can be done wirelessly — no USB cable required.

1. Connect to the `zero-bw16` access point
2. Navigate to `http://192.168.1.1` → **Settings** tab → **OTA Update**
3. Select the new `.bin` file and upload
4. Device reboots automatically into the new firmware

---

### Serial Monitor

```bash
pio device monitor --baud 115200
```

Boot output confirms AP start, scan results, and attack events in real time.

---

## First Use

1. Flash firmware to the BW16 module
2. Connect to the access point:
   - **SSID:** `zero-bw16`
   - **Password:** `zerobw16!`
3. Open a browser and navigate to `http://192.168.1.1`
4. Go to the **Settings** tab and change the AP credentials immediately

---

## Default Credentials

| Setting | Default |
|---|---|
| AP SSID | `zero-bw16` |
| AP Password | `zerobw16!` |
| AP Channel | 6 |
| Web UI | `http://192.168.1.1` |

> Change credentials on first boot. The defaults are public.

---

## Web API Reference

All endpoints accept and return JSON. Base URL: `http://192.168.1.1`

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/scan` | Trigger a WiFi scan |
| `GET` | `/api/networks` | Return scanned network list |
| `POST` | `/api/attack/start` | Start attack with mode and targets |
| `POST` | `/api/attack/stop` | Stop running attack |
| `GET` | `/api/attack/status` | Current attack mode and packet stats |
| `GET` | `/api/sniff/probes` | Captured probe request table |
| `GET` | `/api/sniff/clients` | Client-to-AP association map |
| `GET` | `/api/sniff/pmkid` | Captured PMKIDs |
| `GET` | `/api/sniff/handshakes` | Captured EAPOL handshakes |
| `POST` | `/api/sniff/start` | Start passive sniffer |
| `POST` | `/api/sniff/stop` | Stop passive sniffer |
| `POST` | `/api/beacon/start` | Start beacon spam |
| `POST` | `/api/beacon/stop` | Stop beacon spam |
| `GET` | `/api/log` | Return in-memory attack log |
| `POST` | `/api/log/clear` | Clear log buffer |
| `GET` | `/api/settings` | Return current settings |
| `POST` | `/api/settings` | Update settings |
| `POST` | `/api/settings/reset` | Reset settings to factory defaults |
| `POST` | `/ota` | OTA firmware upload |
| `GET` | `/events` | SSE stream for real-time UI updates |

---

## Author

**Arshad AK**

| | |
|---|---|
| Portfolio | [arshadakl.in](https://www.arshadakl.in) |
| SSH Portfolio | `ssh arshadakl.in` *(open your terminal and try it)* |
| GitHub | [@arshadakl](https://github.com/arshadakl) |

---

## License

GPL-3.0 — see [LICENSE](LICENSE)

Redistribution and use with modification is permitted under the terms of the GPL-3.0 license. Attribution required.
