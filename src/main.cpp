#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "log.h"
#include "system/nvs.h"
#include "system/led.h"
#include "system/button.h"
#include "system/ota.h"
#include "wifi/scanner.h"
#include "wifi/attacker.h"
#include "wifi/sniffer.h"
#include "web/webserver.h"

static void startAP() {
    Settings& cfg = Nvs.cfg();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(cfg.ap_ssid, cfg.ap_pass, cfg.ap_channel);
    Log.log(LOG_INFO, "AP: SSID=%s CH=%d IP=%s",
            cfg.ap_ssid, cfg.ap_channel, AP_IP);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.printf("\n\n=== %s v%s ===\n", PROJECT_NAME, FW_VERSION);

    // Init subsystems
    Log.init();
    Nvs.init();
    Led.init(LED_PIN, true);
    Button.init(BUTTON_PIN, true);

    Log.log(LOG_INFO, "Boot");

    // Start WiFi AP
    startAP();
    delay(500);

    // Init modules
    Scan.init();
    Attack.init();
    Sniff.init();
    WebSrv.init();

    Led.setState(LED_IDLE);
    Log.log(LOG_INFO, "Ready. Connect to '%s' -> %s", Nvs.cfg().ap_ssid, AP_IP);

    // Auto behaviours
    if (Nvs.cfg().auto_scan) {
        Led.setState(LED_SCAN);
        Scan.scan();
        Led.setState(LED_IDLE);
    }
    if (Nvs.cfg().auto_attack && Scan.networkCount() > 0) {
        for (int i = 0; i < Scan.networkCount(); i++) {
            const Network* n = Scan.network(i);
            if (n) Attack.addTarget(n->bssid);
        }
        Attack.startAttack(ATK_DEAUTH);
        Led.setState(LED_ATTACK);
    }
}

void loop() {
    // Handle web requests
    WebSrv.tick();

    // Attack tick
    Attack.tick();

    // LED tick
    Led.tick();

    // Button: short=scan, long=toggle attack
    ButtonEvent btn = Button.poll();
    if (btn == BTN_SHORT) {
        Log.log(LOG_INFO, "Button: scan");
        Led.setState(LED_SCAN);
        Scan.scan();
        Led.setState(Attack.isRunning() ? LED_ATTACK : LED_IDLE);
    } else if (btn == BTN_LONG) {
        if (Attack.isRunning()) {
            Attack.stopAttack();
            Led.setState(LED_IDLE);
            Log.log(LOG_INFO, "Button: stop attack");
        } else if (Scan.networkCount() > 0) {
            Attack.clearTargets();
            for (int i = 0; i < Scan.networkCount(); i++) {
                const Network* n = Scan.network(i);
                if (n) Attack.addTarget(n->bssid);
            }
            Attack.startAttack(ATK_DEAUTH);
            Led.setState(LED_ATTACK);
            Log.log(LOG_INFO, "Button: start attack on %d nets", Scan.networkCount());
        }
    }

    // SSE log push (push new entries to connected SSE client)
    static uint32_t lastLogChange = 0;
    if (Log.changeCounter() != lastLogChange) {
        lastLogChange = Log.changeCounter();
        // Push last log entry via SSE using circular-buffer-aware lastEntry()
        const LogEntry* last = Log.lastEntry();
        if (last) {
            const LogEntry* e = last;
            static const char* lvlNames[] = {"info","warn","error","attack","sniff"};
            char json[160];
            snprintf(json, sizeof(json),
                "{\"type\":\"log\",\"ts\":%lu,\"level\":\"%s\",\"msg\":\"%s\"}",
                e->ts, lvlNames[min((int)e->level, 4)], e->msg);
            WebSrv.pushSSE(json);
        }
    }

    delay(1);
}
