<div align="center">
  <img src="https://raw.githubusercontent.com/arshadakl/assets/refs/heads/main/Zero-BW16-Security-Firmware.png" alt="Zero-BW16 Security Firmware" width="100%">
</div>

<div align="center">

![Platform](https://img.shields.io/badge/Platform-RTL8720DN%20%7C%20BW16-blue)
![Framework](https://img.shields.io/badge/Framework-Arduino-teal)
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

## Hardware

| Spec | Value |
|---|---|
| Module | AI-Thinker BW16 (RTL8720DN) |
| SoC | Realtek RTL8720DN (KM0 + KM4 dual-core ARM) |
| WiFi | 802.11 a/b/g/n/ac — 2.4 GHz + 5 GHz |
| Flash | 2 MB |
| SRAM | 256 KB |
| LED Pin | PA27 (active low) |
| Button Pin | PA12 (optional, pulled high) |
| Power | USB Type-C, 3.3 V |

No external hardware is required. Everything runs on the BW16 module alone.

---

## Build & Flash

> **Important:** PlatformIO does **not** support the RTL8720DN. The only working toolchain is **Arduino IDE** with the official Realtek AmebaD package.

### Requirements

- [Arduino IDE 2.x](https://www.arduino.cc/en/software) (version 2.0 or later)
- Realtek AmebaD board package installed
- USB Type-C cable

### Install Realtek AmebaD Board Package

1. Open Arduino IDE → **File** → **Preferences**
2. Add the following URL to **Additional Boards Manager URLs**:
   ```
   https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek.com_amebad_index.json
   ```
3. Go to **Tools** → **Board** → **Boards Manager**
4. Search for `Realtek AmebaD` and install version **3.1.7** or later
5. Select board: **Tools** → **Board** → **AmebaD ARM (32-bit) Boards** → **AI-Thinker BW16-TypeC**

### Entering Download Mode

**Required before every UART flash.** Do this before clicking Upload:

1. Hold the **BOOT** button on the BW16
2. Press and release **RESET** while holding BOOT
3. Release **BOOT**
4. The module is now in download mode — the LED will be off

### Compile and Flash (Arduino IDE)

1. Open `zero-bw16.ino` in Arduino IDE
2. Select the correct COM port: **Tools** → **Port** → *(your BW16 port, e.g. COM7)*
3. Put the BW16 into download mode (see above)
4. Click **Upload** (or press `Ctrl+U`)
5. Arduino IDE compiles then uploads automatically — wait for `All images are sent successfully!`
6. Press **RESET** after upload completes to boot the new firmware

### Serial Monitor

Open **Tools** → **Serial Monitor**, set baud rate to **115200**.

Boot output confirms AP start, scan results, and attack events in real time.

---

## First Use

1. Flash firmware to the BW16 module
2. Press **RESET** to boot
3. Connect to the access point:
   - **SSID:** `zero-bw16`
   - **Password:** `zerobw16!`
4. Open a browser and navigate to `http://192.168.1.1`
5. Go to the **Settings** tab and change the AP credentials immediately

> **Default credentials are public — change them on first boot.**

| Setting | Default |
|---|---|
| AP SSID | `zero-bw16` |
| AP Password | `zerobw16!` |
| AP Channel | 6 |
| Web UI | `http://192.168.1.1` |

---

## Features & Usage

### Attack Tab

The **Attack** tab is the main interface. It shows all networks discovered in the last scan. Each row shows: SSID, BSSID, channel, RSSI, frequency band, encryption type, WPS/PMF flags, and the number of detected clients.

**Workflow:**
1. Click **Scan** to discover nearby networks
2. Check one or more target networks in the table
3. Select an attack mode from the dropdown
4. Configure parameters (frames, delay, reason code)
5. Click **Launch Attack** — the live packet counter updates in real time
6. Click **Stop** to halt the attack

---

#### Deauthentication Attack

Deauthentication frames are 802.11 management frames that force a client to disconnect from an AP. The BW16 injects these frames as a third party — no association required.

Zero-BW16 supports three deauth modes:

---

**1. Direct (Targeted) Deauth**

- Select one or more APs in the network table
- Select attack mode: **Deauth**
- Optionally expand a row to select a specific client MAC — if no client is selected, all connected clients are targeted via the AP's BSSID as destination

What happens:
- Firmware sends deauth frames from the AP's BSSID to the target client (AP→Client direction)
- Then immediately sends a second deauth spoofing the client back to the AP (Client→AP direction) — this is **bidirectional deauth**
- Both directions together force a full disconnect even on clients that ignore one-directional frames
- The radio switches to the target AP's channel automatically before injection

Use case: Disconnect a specific client from a specific AP for testing reconnection behavior or capturing the WPA2 handshake.

---

**2. Broadcast Deauth**

- Select one or more APs in the network table
- Select attack mode: **Broadcast Deauth**

What happens:
- Firmware sends deauth frames from the AP's BSSID to `FF:FF:FF:FF:FF:FF` (broadcast address)
- All clients associated with the AP receive the frame simultaneously
- More efficient than targeting each client individually — one frame disconnects the entire AP

Use case: Mass-disconnect all clients from an AP. Useful for testing AP behavior under broadcast deauth or capturing multiple handshakes at once.

---

**3. Bidirectional Deauth (automatic)**

Bidirectional is not a separate mode — it is always applied during Direct Deauth when a specific client MAC is targeted. No extra configuration needed.

---

**Parameters:**
- **Frames per burst:** Number of deauth frames sent per tick (default: 3). Higher = more aggressive, more detectable.
- **Delay (ms):** Wait between bursts. Set to 0 for continuous flood.
- **Reason Code:** 802.11 reason code embedded in the deauth frame (default: 1 = Unspecified). Most clients ignore the reason code.

---

#### Auth Flood

Floods a target AP with spoofed 802.11 authentication requests from random MAC addresses.

- Select one or more APs
- Select attack mode: **Auth Flood**
- Launch

What happens: The firmware generates a new random source MAC for every frame, then sends an 802.11 Open System authentication request to the target AP. High-rate floods can exhaust the AP's association table, causing legitimate clients to be refused or slowing down the AP.

---

#### Association Flood

Floods a target AP with spoofed 802.11 association requests.

- Select one or more APs
- Select attack mode: **Assoc Flood**
- Launch

Similar to Auth Flood but sends association requests instead. Requires a known SSID — the firmware uses the SSID captured during the scan. Each frame uses a random spoofed MAC.

---

#### Beacon Spam

Broadcasts fake access point beacons with custom or random SSIDs. Nearby devices see these as real networks.

Navigate to the **Beacon Spam** tab:

1. Enter SSIDs in the text area (one per line) — or check **Randomize** to generate random names
2. Set the broadcast **channel** (1–13)
3. Set the **interval** in milliseconds between beacon bursts
4. Click **Start Beacon Spam**

What happens: The firmware cycles through the SSID list, generates a random BSSID for each, and broadcasts a valid 802.11 beacon frame. The target channel's scan results fill up with fake networks.

Use case: Testing how clients behave when many APs are visible, demonstrating SSID pollution, or generating noise during a red team exercise.

---

#### Evil Twin AP

Clones a target AP — same SSID and channel — to act as a rogue access point. Clients that connect will get DHCP from the BW16 and DNS queries will redirect to the captive portal at `192.168.1.1`.

- Select a target AP from the network table
- Click **Evil Twin** (or select from the dropdown and click Launch)

What happens:
- The BW16's AP interface starts with the cloned SSID and channel
- The firmware stops injecting frames and the AP handles connections passively
- The built-in DNS server answers all queries with `192.168.1.1`
- Victims that connect see the captive portal page

For maximum effectiveness, run a Broadcast Deauth against the real AP simultaneously to push clients off — then they may auto-connect to the Evil Twin.

---

#### Karma Attack

Responds to every 802.11 probe request with a beacon advertising the exact SSID the client probed for.

- Go to the **Attack** tab → select mode **Karma** → Launch

What happens: The sniffer captures probe requests passively. When a client probes for a previously-remembered network (e.g. "HomeWiFi"), the firmware immediately sends a beacon claiming to be "HomeWiFi". The client may auto-connect.

Karma is effective against devices with saved networks that broadcast probe requests looking for them.

---

### Sniff Tab

Passive capture mode. The sniffer puts the radio into promiscuous mode to capture all nearby 802.11 frames without transmitting anything.

Click **Start Sniffing** to begin. Click **Stop** to halt.

---

#### Probe Request Sniffing

Displays a live table of captured probe requests:

| Column | Meaning |
|---|---|
| Source MAC | Device sending the probe |
| Probed SSID | Network the device is looking for (empty = wildcard probe) |
| RSSI | Signal strength |
| First Seen | Timestamp of first capture |
| Count | How many times this probe was seen |

Use this to map which devices are looking for which networks — useful before a Karma attack.

---

#### Client-to-AP Mapping

Captures 802.11 association frames to build a real-time map of which client MACs are connected to which AP BSSIDs.

Displayed as a table: AP BSSID → list of associated client MACs with last-seen timestamp.

This data also populates the **client count** column in the Attack tab's network table.

---

#### PMKID Capture

Captures PMKID values from EAPOL Message 1 frames (the first frame of a WPA2 4-way handshake).

A PMKID is a hash derived from the PMK, AP BSSID, and client MAC. It can be cracked offline without capturing a full handshake — only one packet from the AP is needed, and no client needs to be present.

The captured table shows: BSSID, client MAC, SSID, PMKID (hex), and timestamp.

Export the PMKID and use with hashcat:
```
hashcat -m 22000 pmkid.txt wordlist.txt
```

---

#### WPA2 4-Way Handshake Capture

Tracks all 4 EAPOL frames of the WPA2 handshake per BSSID+client pair. The table shows which messages have been captured (bitmask of frames 1–4), ANonce, SNonce, MIC, and timestamp.

A complete handshake (messages 1+2 minimum) is sufficient for offline cracking.

To trigger a handshake: run a Deauth attack against the target client while the sniffer is running — the client will reconnect and the BW16 captures the handshake automatically.

---

### Log Tab

Shows the in-memory circular event log. Each entry has a timestamp, log level (INFO / ATTACK / SNIFF / ERROR), and message text.

Click **Clear** to reset the log buffer.

Log entries are also printed to the serial port at 115200 baud.

---

### Settings Tab

| Setting | Description |
|---|---|
| **AP SSID** | SSID the BW16 broadcasts as its own access point |
| **AP Password** | WPA2 password for the management AP |
| **AP Channel** | Channel for the management AP (1–13) |
| **Frames per burst** | How many frames to send per attack tick |
| **Send delay (ms)** | Milliseconds between attack ticks |
| **Auto-scan on boot** | Run a WiFi scan automatically at startup |
| **Auto-attack on boot** | Resume the last attack mode automatically at startup |
| **LED enable** | Enable or disable the status LED |
| **Dark mode** | Toggle dark/light UI theme (saved in browser localStorage) |

Click **Save** to persist settings to EEPROM (survives reboot).
Click **Reset to Defaults** to restore factory settings.

> **OTA firmware update is not supported in the current build.** Use Arduino IDE + UART download mode for all firmware updates.

---

### Hardware Button

A GPIO button on pin **PA12** (active low, internal pull-up) provides hardware control without needing the web UI:

| Press | Action |
|---|---|
| Short press (< 1s) | Trigger a WiFi scan |
| Long press (≥ 3s) | Toggle attack on/off (uses last configured mode and targets) |

---

### LED Patterns

The LED on pin **PA27** (active low) indicates device state:

| Pattern | State |
|---|---|
| Slow blink (1 Hz) | Idle — AP running, no active attack |
| Fast blink (5 Hz) | Scan in progress |
| Solid on | Attack running |
| Double blink, pause | Error condition |
| Off | Booting or download mode |

LED can be disabled in the Settings tab.

---

## Features at a Glance

### Active Attacks

| Feature | Mode String | Description |
|---|---|---|
| Targeted Deauth | `deauth` | Disconnect specific clients from specific APs |
| Broadcast Deauth | `deauth_bcast` | Disconnect all clients from an AP simultaneously |
| Auth Flood | `auth_flood` | Exhaust AP association table with spoofed auth requests |
| Assoc Flood | `assoc_flood` | Flood AP with spoofed association requests |
| Beacon Spam | `beacon_spam` | Flood scan results with fake SSIDs |
| Evil Twin | `evil_twin` | Clone target AP as rogue access point with captive portal |
| Karma Attack | `karma` | Auto-respond to probe requests with matching fake AP |

### Passive Intelligence

| Feature | Description |
|---|---|
| Probe Sniffing | Capture device probe requests with source MAC, SSID, RSSI |
| Client Mapping | Map client MACs to AP BSSIDs from association frames |
| WPS Detection | Flag APs with WPS enabled (parsed from beacon IEs) |
| PMF Detection | Flag APs with Management Frame Protection (RSN IE) |
| PMKID Extraction | Extract PMKID from EAPOL msg1 for offline cracking |
| Handshake Capture | Capture WPA2 4-way EAPOL handshake (msgs 1–4) |
| RSSI History | Rolling 10-sample RSSI per BSSID |
| Hidden SSID Track | Log APs broadcasting empty SSIDs |

### System

| Feature | Description |
|---|---|
| Dual-Band | 2.4 GHz (ch 1–13) + 5 GHz scan support |
| Real-Time UI | Server-Sent Events push updates without page refresh |
| NVS Persistence | All settings saved to EEPROM, survive reboot |
| Captive Portal | DNS server redirects all queries to 192.168.1.1 |
| Circular Log | 40-entry in-memory event log with levels and timestamps |
| Hardware Button | Scan (short) / attack toggle (long) |
| LED State Machine | Idle / scan / attack / error patterns |

---

## Project Structure

```
src/
├── main.cpp                  — setup() and loop(), boot logic, SSE push
├── config.h                  — compile-time defaults, limits, 5GHz channel table
├── log.cpp / log.h           — circular buffer logger, 5 log levels
├── wifi/
│   ├── scanner.cpp/h         — dual-band scan via RTK SDK, RSSI history, client count
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
    └── ota.cpp/h             — OTA stub (not supported on RTL8720DN Arduino SDK)
```

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
| `GET` | `/events` | SSE stream for real-time UI updates |

---

## Contributing

Contributions are welcome. This project is open-source under GPL-3.0.

### Guidelines

- **Scope:** BW16 (RTL8720DN) only — no external hardware dependencies
- **Toolchain:** Arduino IDE + AmebaD package only — do not add PlatformIO configs
- **Code style:** Match existing style; no unnecessary abstractions
- **Commits:** Use conventional commits — `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`
- **Testing:** Verify on real hardware before submitting
- **New attacks / features:** Open an issue first before implementing

### How to Contribute

1. Fork the repository
2. Create a feature branch: `git checkout -b feat/your-feature`
3. Make changes and commit with conventional commit messages
4. Push to your fork: `git push origin feat/your-feature`
5. Open a Pull Request against `main`

---

## Author

**Arshad AKL**

| | |
|---|---|
| Portfolio | [arshadakl.in](https://www.arshadakl.in) |
| SSH Portfolio | `ssh arshadakl.in` *(open your terminal and try it)* |

---

## License

GPL-3.0 — see [LICENSE](LICENSE)

Redistribution and use with modification is permitted under the terms of the GPL-3.0 license. Attribution required.
