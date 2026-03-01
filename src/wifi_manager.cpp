#include "wifi_manager.h"
#include "project_config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include <Preferences.h>

extern String device_id;
static bool is_config_mode = false;

static void configModeCallback(WiFiManager *myWiFiManager) {
    is_config_mode = true;
    Serial.println("Entered config mode");
}

void wifi_init(void) {
    // First, try to use saved credentials from Preferences (NVS).
    Preferences prefs;
    if (prefs.begin("wifi", true)) {
        String saved_ssid = prefs.getString("ssid", "");
        String saved_pass = prefs.getString("pass", "");
        prefs.end();

        if (saved_ssid.length() > 0) {
            Serial.printf("Attempting to connect using saved SSID: %s\n", saved_ssid.c_str());
            WiFi.begin(saved_ssid.c_str(), saved_pass.c_str());
            unsigned long start = millis();
            const unsigned long timeout = 8000; // 8s
            while (millis() - start < timeout) {
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("Connected using saved WiFi credentials.");
                    is_config_mode = false;
                    String hostname = String(MDNS_HOSTNAME_PREFIX) + device_id;
                    if (MDNS.begin(hostname.c_str())) {
                        MDNS.addService("http", "tcp", 80);
                        Serial.printf("mDNS responder started: %s.local\n", hostname.c_str());
                    }
                    return;
                }
                delay(250);
            }
            Serial.println("Failed to connect with saved credentials, falling back to WiFiManager.");
        }
    }

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
    // Also clear saved credentials from Preferences
    Preferences prefs;
    if (prefs.begin("wifi", false)) {
        prefs.clear();
        prefs.end();
        Serial.println("Cleared saved WiFi credentials from Preferences.");
    }
}

bool wifi_is_sta_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}

bool wifi_is_config_mode(void) {
    return is_config_mode;
}

bool wifi_save_sta_config(const char* ssid, const char* pass) {
    // Persist credentials to NVS (Preferences) and try to connect.
    // Note: NVS is not encrypted. Avoid storing highly sensitive secrets if
    // you need stronger security.
    Serial.printf("Saving WiFi credentials: ssid=%s\n", ssid);

    // Basic validation
    if (ssid == nullptr || strlen(ssid) == 0) {
        Serial.println("Empty SSID provided, aborting save.");
        return false;
    }

    Preferences prefs;
    if (!prefs.begin("wifi", false)) {
        Serial.println("Failed to open Preferences namespace 'wifi'");
        return false;
    }

    // Save values
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass ? pass : "");
    prefs.end();

    // Try to connect using provided credentials
    WiFi.disconnect(true, true);
    WiFi.begin(ssid, pass);
    unsigned long start = millis();
    const unsigned long timeout = 10000; // 10s
    while (millis() - start < timeout) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to WiFi after saving credentials.");
            return true;
        }
        delay(250);
    }

    Serial.println("Failed to connect with provided credentials within timeout.");
    return false;
}
