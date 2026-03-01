#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "project_config.h"

void sensors_init(uint8_t soil_pin, uint8_t dht_pin, uint8_t sda, uint8_t scl);
void sensors_update_state(sensor_data_t *data);
sensor_data_t sensors_get_latest_data(void);

int sensors_read_soil_moisture(void);
bool sensors_read_dht22(float *temperature, float *humidity);
bool sensors_read_air_quality(uint16_t *co2, uint16_t *tvoc);
bool sensors_are_healthy(void);

#endif
