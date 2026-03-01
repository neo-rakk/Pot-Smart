#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <Arduino.h>

// WiFi Configuration (SoftAP for WiFiManager)
#define AP_SSID_PREFIX "Pot-Smart-Config-"

// mDNS Configuration
#define MDNS_HOSTNAME_PREFIX "pot-"

// Hardware Pinout (Arduino Pins)
#define RELAY_GPIO 26
#define SOIL_ADC_CH 34 // GPIO34
#define DHT22_GPIO 14

// Consolidated I2C
#define I2C_SDA_GPIO 21
#define I2C_SCL_GPIO 22

// Status LED & Reset Button
#define STATUS_LED_GPIO 2
#define RESET_BUTTON_GPIO 0

// Soil Moisture Calibration (Example values: raw readings for Air and Water)
#define SOIL_AIR_VALUE 3200
#define SOIL_WATER_VALUE 1500

typedef struct {
    float temperature;
    float humidity;
    int soil_moisture;
    uint16_t co2;
    uint16_t tvoc;
} sensor_data_t;

typedef struct {
    int base_threshold_min;
    int calculated_threshold_min;
    char climate_mode[16];
} logic_data_t;

#endif // PROJECT_CONFIG_H
