# zero-bw16

Advanced WiFi security research firmware for the **AI-Thinker BW16 (RTL8720DN)** module.

> For authorized penetration testing, CTF competitions, and WiFi security research only.

---

## Features

### Attacks
- **Deauth** — directed or broadcast deauthentication
- **Auth Flood** — flood AP with spoofed auth requests
- **Assoc Flood** — flood AP with spoofed association requests
- **Beacon Spam** — flood area with custom/random fake SSIDs
- **Evil Twin** — clone target AP SSID + channel
- **Karma Attack** — respond to all probe requests with matching SSID

### Passive Intelligence
- **Probe Sniffer** — capture nearby device probe requests
- **Client Mapper** — map associated clients to each AP
- **PMKID Capture** — extract PMKID from EAPOL msg1 for offline cracking
- **Handshake Capture** — capture WPA2 4-way EAPOL handshake

### Web Interface
- Single-page app at `192.168.1.1`
- 5 tabs: Attack / Sniff / Beacon / Log / Settings
- Dark and light mode
- Real-time SSE updates (no page refresh needed)
- Drag-and-drop OTA firmware update

### System
- NVS settings persistence (survives reboot)
- Auto-scan and auto-attack on boot flags
- Hardware button: short=scan, long=toggle attack
- LED feedback: idle / scan / attack / error patterns
- DNS captive portal (all DNS → 192.168.1.1)

---

## Hardware

| Component | Value |
|---|---|
| Module | AI-Thinker BW16 (RTL8720DN) |
| WiFi | 2.4 GHz 802.11 b/g/n |
| Flash | 2 MB |
| RAM | 256 KB SRAM + PSRAM |
| LED pin | PA27 (active low) |
| Button pin | PA12 (optional) |

---

## Build & Flash

### Requirements
- [PlatformIO](https://platformio.org/) with `realtek-ambz2` platform
- USB Type-C cable

### Build
```bash
pio run
```

### Flash
```bash
pio run -t upload
```
Or use the GUI flasher tool from the Ereshkigal project.

### Connect
1. Connect to WiFi: `zero-bw16` / `zerobw16!`
2. Open browser: `http://192.168.1.1`

---

## Default Credentials

| Setting | Default |
|---|---|
| AP SSID | `zero-bw16` |
| AP Password | `zerobw16!` |
| AP Channel | 6 |
| Web UI | `192.168.1.1` |

Change all credentials in the Settings tab after first boot.

---

## License

GPL-3.0 — see [LICENSE](LICENSE)
