#include <Arduino.h>
#include "project_config.h"
#include "storage.h"
#include "sensors.h"
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
String device_id;

void task_led_status(void *pv) {
    for(;;) {
        if (!sensors_are_healthy()) {
            // Double Flash: Sensor Error
            digitalWrite(STATUS_LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(STATUS_LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(STATUS_LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(STATUS_LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else if (wifi_is_config_mode()) {
            // Fast blink: Configuration Mode
            digitalWrite(STATUS_LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(STATUS_LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else if (!wifi_is_sta_connected()) {
            // Slow blink: WiFi search
            digitalWrite(STATUS_LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(1000));
            digitalWrite(STATUS_LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            // Steady: Connected & Operational
            digitalWrite(STATUS_LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void task_sensors(void *pv) {
    sensor_data_t data;
    esp_task_wdt_add(NULL);
    for(;;) {
        esp_task_wdt_reset();
        data.soil_moisture = sensors_read_soil_moisture();
        bool got = sensors_read_dht22(&data.temperature, &data.humidity);
        sensors_read_air_quality(&data.co2, &data.tvoc);
        // Debug: print sensor readings to Serial for visibility
        if (got) {
            Serial.printf("Sensors: T=%.1f C, H=%.1f %%, Soil=%d%%, eCO2=%u ppm, TVOC=%u ppb\n",
                data.temperature, data.humidity, data.soil_moisture, data.co2, data.tvoc);
        } else {
            Serial.printf("Sensors: DHT read failed, Soil=%d%%, eCO2=%u ppb, TVOC=%u ppb\n",
                data.soil_moisture, data.co2, data.tvoc);
        }
        sensors_update_state(&data);
        xQueueSend(sensor_data_queue, &data, pdMS_TO_TICKS(1000));
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void setup() {
    Serial.begin(115200);
    esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);

    // Get DeviceID
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id_str[7];
    snprintf(id_str, sizeof(id_str), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    device_id = String(id_str);
    Serial.printf("DeviceID: %s\n", device_id.c_str());

    i2c_mutex = xSemaphoreCreateMutex();
    arrosage_init(); // Initialize logic mutex early

    // Wire initialization
    Wire.begin(I2C_SDA_GPIO, I2C_SCL_GPIO);

    storage_init();
    sensors_init(SOIL_ADC_CH, DHT22_GPIO, I2C_SDA_GPIO, I2C_SCL_GPIO);
    relay_init(RELAY_GPIO);
    pinMode(STATUS_LED_GPIO, OUTPUT);
    pinMode(RESET_BUTTON_GPIO, INPUT_PULLUP);

    sensor_data_queue = xQueueCreate(10, sizeof(sensor_data_t));

    // Start LED task BEFORE WiFi initialization to provide feedback during provisioning
    xTaskCreate(task_led_status, "led", 2048, NULL, 2, NULL);

    wifi_init();
    start_webserver();

    xTaskCreate(task_sensors, "sensors", 4096, NULL, 5, NULL);
    xTaskCreate(arrosage_task, "arrosage", 4096, NULL, 5, NULL);
}

static unsigned long reset_press_start = 0;

void loop() {
    server.handleClient();

    // Reset Button logic (GPIO 0 is active low)
    if (digitalRead(RESET_BUTTON_GPIO) == LOW) {
        if (reset_press_start == 0) {
            reset_press_start = millis();
        } else if (millis() - reset_press_start > 5000) {
            Serial.println("Reset Button held for 5s. Clearing WiFi settings...");
            wifi_reset_settings();
            ESP.restart();
        }
    } else {
        reset_press_start = 0;
    }

    vTaskDelay(1);
}
