#ifndef PLANT_CONFIG_H
#define PLANT_CONFIG_H

#include <Arduino.h>

typedef struct {
    char plant_name[32];
    int current_min_humidity;
    int current_max_humidity;
    int pump_duration_ms;
    bool auto_mode;
} plant_config_t;

typedef struct {
    const char *name;
    int default_min_humidity;
    int default_max_humidity;
} plant_profile_t;

void plant_config_init(void);
void update_plant_config(const char *plant, int min, int max, bool auto_mode);
plant_config_t get_plant_config(void);
const plant_profile_t* get_predefined_plants(int *count);

#endif
