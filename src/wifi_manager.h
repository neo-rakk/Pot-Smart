#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void wifi_init(void);
bool wifi_is_sta_connected(void);
bool wifi_is_config_mode(void);
void wifi_reset_settings(void);
// Save STA credentials to NVS (Preferences) and attempt to connect.
// Returns true if credentials were saved and the ESP connected to the AP.
bool wifi_save_sta_config(const char* ssid, const char* pass);

#endif
