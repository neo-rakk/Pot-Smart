#include "sensors.h"
#include <Wire.h>
#include <DHTesp.h>
#include <Adafruit_SGP30.h>

static uint8_t g_soil_pin, g_dht_pin;
static sensor_data_t latest_data = {0};
static SemaphoreHandle_t data_mutex;
static DHTesp dht;
static Adafruit_SGP30 sgp;
static bool sgp_present = false;

extern SemaphoreHandle_t i2c_mutex;

void sensors_init(uint8_t soil_pin, uint8_t dht_pin, uint8_t sda, uint8_t scl) {
    g_soil_pin = soil_pin;
    g_dht_pin = dht_pin;
    data_mutex = xSemaphoreCreateMutex();

    Serial.printf("sensors_init: initializing DHT on pin %u\n", g_dht_pin);
    dht.setup(g_dht_pin, DHTesp::DHT22);
    Serial.printf("DHT model: %d, min sampling period: %d ms\n", (int)dht.getModel(), dht.getMinimumSamplingPeriod());

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        if (!sgp.begin()) {
            Serial.println("SGP30 sensor not found :(");
            sgp_present = false;
        } else {
            Serial.print("Found SGP30 serial #");
            Serial.print(sgp.serialnumber[0], HEX);
            Serial.print(sgp.serialnumber[1], HEX);
            Serial.println(sgp.serialnumber[2], HEX);
            sgp_present = true;
        }
        xSemaphoreGive(i2c_mutex);
    }
}

int sensors_read_soil_moisture(void) {
    int raw = analogRead(g_soil_pin);
    int moisture = map(raw, SOIL_AIR_VALUE, SOIL_WATER_VALUE, 0, 100);
    return (moisture < 0) ? 0 : (moisture > 100) ? 100 : moisture;
}

bool sensors_read_dht22(float *temperature, float *humidity) {
    const int max_attempts = 3;
    const int retry_delay_ms = dht.getMinimumSamplingPeriod(); // use sensor's minimum sampling period (usually 2000ms for DHT22)
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        TempAndHumidity values = dht.getTempAndHumidity();
        DHTesp::DHT_ERROR_t status = dht.getStatus();
        if (status == DHTesp::ERROR_NONE) {
            *temperature = values.temperature;
            *humidity = values.humidity;
            if (attempt > 1) {
                Serial.printf("DHT read success on attempt %d\n", attempt);
            }
            return true;
        }
        Serial.printf("DHT read attempt %d error: %s\n", attempt, dht.getStatusString());
        // Prepare for next attempt
        dht.resetTimer();
        if (attempt < max_attempts) delay(retry_delay_ms);
    }
    return false;
}

bool sensors_read_air_quality(uint16_t *co2, uint16_t *tvoc) {
    if (!sgp_present) return false;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        bool success = sgp.IAQmeasure();
        if (success) {
            *co2 = sgp.eCO2;
            *tvoc = sgp.TVOC;
        }
        xSemaphoreGive(i2c_mutex);
        return success;
    }
    return false;
}

bool sensors_are_healthy(void) {
    float t, h;
    sensors_read_dht22(&t, &h);
    bool dht_ok = (dht.getStatus() == DHTesp::ERROR_NONE);
    return dht_ok && sgp_present;
}

void sensors_update_state(sensor_data_t *data) {
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100))) {
        latest_data = *data;
        xSemaphoreGive(data_mutex);
    }
}

sensor_data_t sensors_get_latest_data(void) {
    sensor_data_t data = {0};
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100))) {
        data = latest_data;
        xSemaphoreGive(data_mutex);
    }
    return data;
}
