#include "plant_config.h"
#include "storage.h"

static const plant_profile_t predefined_plants[] = {
    {"Pitaya", 60, 80},
    {"Cactus", 10, 30},
    {"Fern", 50, 70},
    {"Orchid", 40, 60},
    {"Bonsai", 30, 50},
    {"Tomato", 55, 75},
    {"Basil", 50, 70},
    {"Succulent", 15, 35},
    {"Rose", 45, 65},
    {"Lavender", 20, 40},
    {"Custom", 0, 0}
};
static const int num_predefined_plants = sizeof(predefined_plants) / sizeof(predefined_plants[0]);

// C++ (pre C++20 compilers used here) does not support C99 designated initializers.
// Use positional initialization matching the order in `plant_config_t`.
static plant_config_t current_config = {
    "Pitaya",
    60,
    80,
    5000,
    true
};

void plant_config_init(void) {
    strncpy(current_config.plant_name, predefined_plants[0].name, sizeof(current_config.plant_name) - 1);
    current_config.current_min_humidity = predefined_plants[0].default_min_humidity;
    current_config.current_max_humidity = predefined_plants[0].default_max_humidity;
    current_config.auto_mode = true;
}

void update_plant_config(const char *plant_name_or_custom, int min_h, int max_h, bool new_auto_mode) {
    bool found = false;
    if (plant_name_or_custom && strlen(plant_name_or_custom) > 0) {
        for (int i = 0; i < num_predefined_plants; ++i) {
            if (strcmp(plant_name_or_custom, predefined_plants[i].name) == 0 && strcmp(plant_name_or_custom, "Custom") != 0) {
                strncpy(current_config.plant_name, predefined_plants[i].name, sizeof(current_config.plant_name) - 1);
                current_config.current_min_humidity = predefined_plants[i].default_min_humidity;
                current_config.current_max_humidity = predefined_plants[i].default_max_humidity;
                found = true;
                break;
            }
        }
    }
    if (!found) {
        strncpy(current_config.plant_name, plant_name_or_custom ? plant_name_or_custom : "Custom", sizeof(current_config.plant_name) - 1);
        current_config.current_min_humidity = min_h;
        current_config.current_max_humidity = max_h;
    }
    current_config.auto_mode = new_auto_mode;
    save_config_to_nvs(&current_config);
}

plant_config_t get_plant_config(void) {
    return current_config;
}

const plant_profile_t* get_predefined_plants(int *count) {
    if (count) *count = num_predefined_plants;
    return predefined_plants;
}
