#include "wifi_manager.h"
#include "project_config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>

static bool sta_connected = false;
static Preferences wifi_prefs;

void wifi_init(void) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);

    wifi_prefs.begin("wifi_sta", true);
    String ssid = wifi_prefs.getString("ssid", "");
    String pass = wifi_prefs.getString("pass", "");
    wifi_prefs.end();

    if (ssid != "") {
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial.println("Connecting to home WiFi...");
    }

    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS responder started: %s.local\n", MDNS_HOSTNAME);
    }
}

bool wifi_save_sta_config(const char* ssid, const char* pass) {
    wifi_prefs.begin("wifi_sta", false);
    wifi_prefs.putString("ssid", ssid);
    wifi_prefs.putString("pass", pass);
    wifi_prefs.end();
    return true;
}

bool wifi_is_sta_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}
