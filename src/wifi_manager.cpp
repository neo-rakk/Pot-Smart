#include "wifi_manager.h"
#include "project_config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

extern String device_id;
static bool is_config_mode = false;

static void configModeCallback(WiFiManager *myWiFiManager) {
    is_config_mode = true;
    Serial.println("Entered config mode");
}

void wifi_init(void) {
    WiFiManager wm;
    wm.setAPCallback(configModeCallback);
    wm.setConfigPortalTimeout(180); // 3 minutes timeout

    // Set custom AP Name with DeviceID
    String ap_name = String(AP_SSID_PREFIX) + device_id;
    Serial.printf("Starting WiFiManager with AP: %s\n", ap_name.c_str());

    if(!wm.autoConnect(ap_name.c_str())) {
        Serial.println("Failed to connect or hit timeout");
        is_config_mode = false;
        // In some cases we might want to continue without WiFi or restart
        // ESP.restart();
    }

    is_config_mode = false;
    Serial.println("Connected to WiFi!");

    String hostname = String(MDNS_HOSTNAME_PREFIX) + device_id;
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS responder started: %s.local\n", hostname.c_str());
    }
}

void wifi_reset_settings(void) {
    WiFiManager wm;
    wm.resetSettings();
}

bool wifi_is_sta_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}

bool wifi_is_config_mode(void) {
    return is_config_mode;
}
