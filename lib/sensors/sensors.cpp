#include "sensors.h"
#include <Wire.h>
#include <DHTesp.h>

static uint8_t g_soil_pin, g_dht_pin;
static sensor_data_t latest_data = {0};
static SemaphoreHandle_t data_mutex;
static DHTesp dht;
extern SemaphoreHandle_t i2c_mutex;

// TwoWire instance used for SGP30 (may be Wire or Wire1 depending on setup)
static TwoWire *sgpWire = &Wire;

#define SGP30_I2C_ADDR 0x58

static uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

void sensors_init(uint8_t soil_pin, uint8_t dht_pin, uint8_t sda, uint8_t scl) {
    g_soil_pin = (soil_pin == 6) ? 34 : soil_pin;
    g_dht_pin = dht_pin;
    data_mutex = xSemaphoreCreateMutex();

    // Wire.begin is assumed to be called in setup()
    Serial.printf("sensors_init: initializing DHT on pin %u\n", g_dht_pin);
    dht.setup(g_dht_pin, DHTesp::DHT22);
    // report initial status
    Serial.printf("DHT status after setup: %d (%s)\n", (int)dht.getStatus(), dht.getStatusString());

    // Choose the TwoWire instance to use for the SGP30 based on the pins
    // passed by the caller. If the pins match the SGP30 pins defined in
    // project_config.h we assume Wire1 was initialized for that sensor.
    if (sda == SGP30_SDA_GPIO && scl == SGP30_SCL_GPIO) {
        sgpWire = &Wire1;
        Serial.println("sensors_init: using Wire1 for SGP30");
    } else {
        sgpWire = &Wire;
        Serial.println("sensors_init: using Wire for SGP30");
    }

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        sgpWire->beginTransmission(SGP30_I2C_ADDR);
        sgpWire->write(0x20); sgpWire->write(0x03);
        sgpWire->endTransmission();
        xSemaphoreGive(i2c_mutex);
    }
}

int sensors_read_soil_moisture(void) {
    int raw = analogRead(g_soil_pin);
    int moisture = (100 * (4095 - raw)) / 4095;
    return (moisture < 0) ? 0 : (moisture > 100) ? 100 : moisture;
}

bool sensors_read_dht22(float *temperature, float *humidity) {
    // Try multiple times to work around transient checksum/time issues.
    const int max_attempts = 3;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        TempAndHumidity values = dht.getTempAndHumidity();
        int st = dht.getStatus();
        if (st == DHTesp::ERROR_NONE) {
            *temperature = values.temperature;
            *humidity = values.humidity;
            if (attempt > 1) Serial.printf("sensors_read_dht22: succeeded on attempt %d\n", attempt);
            return true;
        }
        Serial.printf("sensors_read_dht22: attempt %d failed, status=%d\n", attempt, st);
        // Small delay before retrying. Keep it short to avoid blocking other duties.
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    // All attempts failed
    return false;
}

bool sensors_read_air_quality(uint16_t *co2, uint16_t *tvoc) {
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        sgpWire->beginTransmission(SGP30_I2C_ADDR);
        sgpWire->write(0x20); sgpWire->write(0x08);
        if (sgpWire->endTransmission() != 0) {
            xSemaphoreGive(i2c_mutex);
            return false;
        }

        delay(20);
        sgpWire->requestFrom(SGP30_I2C_ADDR, (uint8_t)6);
        if (sgpWire->available() == 6) {
            uint8_t data[6];
            for(int i=0; i<6; i++) data[i] = sgpWire->read();
            xSemaphoreGive(i2c_mutex);
            if (crc8(data, 2) == data[2] && crc8(data + 3, 2) == data[5]) {
                *co2 = (data[0] << 8) | data[1];
                *tvoc = (data[3] << 8) | data[4];
                return true;
            }
        } else {
            xSemaphoreGive(i2c_mutex);
        }
    }
    return false;
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
