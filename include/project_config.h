#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <Arduino.h>

// WiFi Configuration (SoftAP)
#define AP_SSID "PotConnecte-Config"
#define AP_PASS "admin1234"

// mDNS Configuration
#define MDNS_HOSTNAME "pot-connecte"
#define MDNS_INSTANCE "ESP32_Watering_Pot"

// Hardware Pinout (Arduino Pins)
#define RELAY_GPIO 26
#define SOIL_ADC_CH 34 // GPIO34
#define DHT22_GPIO 14
#define OLED_I2C_PORT I2C_NUM_1
#define OLED_SDA_GPIO 21
#define OLED_SCL_GPIO 22
#define SGP30_I2C_PORT I2C_NUM_0
#define SGP30_SDA_GPIO 4
#define SGP30_SCL_GPIO 5

typedef struct {
    float temperature;
    float humidity;
    int soil_moisture;
    uint16_t co2;
    uint16_t tvoc;
} sensor_data_t;

#endif // PROJECT_CONFIG_H
