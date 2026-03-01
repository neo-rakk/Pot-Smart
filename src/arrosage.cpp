#include "arrosage.h"
#include "project_config.h"
#include "relay.h"
#include "plant_config.h"
#include "display.h"

extern QueueHandle_t sensor_data_queue;

void arrosage_task(void *pvParameters) {
    sensor_data_t data;
    plant_config_t cfg;
    for(;;) {
        cfg = get_plant_config();
        if (xQueueReceive(sensor_data_queue, &data, portMAX_DELAY)) {
            display_status(data.soil_moisture, data.temperature, data.humidity, data.co2, data.tvoc, cfg.current_min_humidity, cfg.current_max_humidity);
            if (cfg.auto_mode) {
                if (data.soil_moisture < cfg.current_min_humidity) {
                    if (!relay_is_on()) relay_on();
                } else if (data.soil_moisture > cfg.current_max_humidity) {
                    if (relay_is_on()) relay_off();
                }
            }
        }
    }
}
