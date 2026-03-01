#include "arrosage.h"
#include "project_config.h"
#include "relay.h"
#include "plant_config.h"
#include <Arduino.h>

extern QueueHandle_t sensor_data_queue;
static logic_data_t current_logic = {0, 0, "NORMAL"};
static SemaphoreHandle_t logic_mutex = NULL;

void arrosage_init(void) {
    if (logic_mutex == NULL) {
        logic_mutex = xSemaphoreCreateMutex();
    }
}

void arrosage_task(void *pvParameters) {
    sensor_data_t data;
    plant_config_t cfg;

    // Ensure init is called, though it should be in setup()
    arrosage_init();

    for(;;) {
        cfg = get_plant_config();
        if (xQueueReceive(sensor_data_queue, &data, portMAX_DELAY)) {
            // Smart-Climate Logic
            int base_threshold = cfg.current_min_humidity;
            int dynamic_threshold = base_threshold;
            String mode = "NORMAL";

            // Dry Factor: Air_Hum < 30% -> +10%
            if (data.humidity < 30.0f) {
                dynamic_threshold += 10;
                mode = "DRY";
            }
            // Heat Factor: Air_Temp > 30°C -> +5%
            if (data.temperature > 30.0f) {
                dynamic_threshold += 5;
                if (mode == "DRY") mode = "DRY_HOT";
                else mode = "HOT";
            }
            // Humid Factor: Air_Hum > 80% -> -5%
            if (data.humidity > 80.0f) {
                dynamic_threshold -= 5;
                mode = "HUMID";
            }

            // Safety Clamping: 10% to 90%
            if (dynamic_threshold < 10) dynamic_threshold = 10;
            if (dynamic_threshold > 90) dynamic_threshold = 90;

            // Update shared logic data
            if (xSemaphoreTake(logic_mutex, pdMS_TO_TICKS(100))) {
                current_logic.base_threshold_min = base_threshold;
                current_logic.calculated_threshold_min = dynamic_threshold;
                strncpy(current_logic.climate_mode, mode.c_str(), sizeof(current_logic.climate_mode) - 1);
                xSemaphoreGive(logic_mutex);
            }

            if (cfg.auto_mode) {
                if (data.soil_moisture < dynamic_threshold) {
                    if (!relay_is_on()) relay_on();
                } else if (data.soil_moisture > cfg.current_max_humidity) {
                    // We keep current_max_humidity as upper bound for stopping.
                    if (relay_is_on()) relay_off();
                }
            }
        }
    }
}

logic_data_t arrosage_get_logic_data(void) {
    logic_data_t data = {0};
    if (logic_mutex && xSemaphoreTake(logic_mutex, pdMS_TO_TICKS(100))) {
        data = current_logic;
        xSemaphoreGive(logic_mutex);
    }
    return data;
}
