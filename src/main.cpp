#include <Arduino.h>
#include "project_config.h"
#include "storage.h"
#include "sensors.h"
#include "display.h"
#include "relay.h"
#include "plant_config.h"
#include "web_server.h"
#include "api.h"
#include "arrosage.h"
#include "wifi_manager.h"
#include <esp_task_wdt.h>
#include <Wire.h>

#define TAG_MAIN "MAIN_APP"
#define WDT_TIMEOUT_MS 30000

QueueHandle_t sensor_data_queue;
SemaphoreHandle_t i2c_mutex;

void task_sensors(void *pv) {
    sensor_data_t data;
    esp_task_wdt_add(NULL);
    for(;;) {
        esp_task_wdt_reset();
        data.soil_moisture = sensors_read_soil_moisture();
        sensors_read_dht22(&data.temperature, &data.humidity);
        sensors_read_air_quality(&data.co2, &data.tvoc);
        sensors_update_state(&data);
        xQueueSend(sensor_data_queue, &data, pdMS_TO_TICKS(1000));
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void setup() {
    Serial.begin(115200);
    esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);

    i2c_mutex = xSemaphoreCreateMutex();

    // Wire initialization
    // Use default TwoWire (Wire) for the OLED so U8g2 HW I2C uses the same bus
    // as the library expects. Put the SGP30 on Wire1.
    Wire.begin(OLED_SDA_GPIO, OLED_SCL_GPIO);
    Wire1.begin(SGP30_SDA_GPIO, SGP30_SCL_GPIO);

    storage_init();
    sensors_init(SOIL_ADC_CH, DHT22_GPIO, SGP30_SDA_GPIO, SGP30_SCL_GPIO);
    // cast pin numbers to gpio_num_t for display_init which expects gpio_num_t
    display_init((gpio_num_t)OLED_SDA_GPIO, (gpio_num_t)OLED_SCL_GPIO, OLED_I2C_PORT);
    relay_init(RELAY_GPIO);

    sensor_data_queue = xQueueCreate(10, sizeof(sensor_data_t));
    wifi_init();
    start_webserver();

    xTaskCreate(task_sensors, "sensors", 4096, NULL, 5, NULL);
    xTaskCreate(arrosage_task, "arrosage", 4096, NULL, 5, NULL);
}

void loop() {
    server.handleClient();
    vTaskDelay(1);
}
