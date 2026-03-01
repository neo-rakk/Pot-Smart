#ifndef DISPLAY_H
#define DISPLAY_H

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Initializes the OLED display on the specified I2C bus and pins.
 *
 * @param sda_pin GPIO number for I2C SDA
 * @param scl_pin GPIO number for I2C SCL
 * @param i2c_port I2C port number (0 or 1)
 * @return esp_err_t ESP_OK if success, or ESP error code
 */
esp_err_t display_init(gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t i2c_port);

/**
 * @brief Displays a simple message on the OLED.
 *
 * @param message String message to display
 */
void display_message(const char* message);

/**
 * @brief Displays the current system status and sensor data.
 */
void display_status(float soil_moisture, float current_temp, float current_humidity,
                    uint16_t co2, uint16_t tvoc,
                    int plant_soil_min_threshold, int plant_soil_max_threshold);

#endif
