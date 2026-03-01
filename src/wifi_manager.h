#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void wifi_init(void);
bool wifi_save_sta_config(const char* ssid, const char* pass);
bool wifi_is_sta_connected(void);

#endif
