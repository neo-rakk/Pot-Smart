#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void wifi_init(void);
bool wifi_is_sta_connected(void);
bool wifi_is_config_mode(void);
void wifi_reset_settings(void);

#endif
