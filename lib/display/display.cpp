#include "display.h"
#include <U8g2lib.h>
#include <Wire.h>

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2 = NULL;
extern SemaphoreHandle_t i2c_mutex;

esp_err_t display_init(gpio_num_t sda, gpio_num_t scl, i2c_port_t port) {
    // Wire instance based on port
    TwoWire *wire = (port == I2C_NUM_1) ? &Wire1 : &Wire;

    u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, scl, sda);

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(500))) {
        u8g2->begin();
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_ncenB08_tr);
        u8g2->drawStr(0, 20, "INIT...");
        u8g2->sendBuffer();
        xSemaphoreGive(i2c_mutex);
    }

    return ESP_OK;
}

void display_message(const char* m) {
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_ncenB08_tr);
        u8g2->drawStr(0, 30, m);
        u8g2->sendBuffer();
        xSemaphoreGive(i2c_mutex);
    }
}

void display_status(float s, float t, float h, uint16_t c, uint16_t v, int min, int max) {
    char buf[32];
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100))) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_6x10_tf);

        snprintf(buf, sizeof(buf), "SOL: %.0f%%", s);
        u8g2->drawStr(0, 10, buf);

        snprintf(buf, sizeof(buf), "TEMP: %.1fC", t);
        u8g2->drawStr(0, 22, buf);

        snprintf(buf, sizeof(buf), "HUM: %.0f%%", h);
        u8g2->drawStr(0, 34, buf);

        snprintf(buf, sizeof(buf), "CO2: %u", c);
        u8g2->drawStr(0, 46, buf);

        u8g2->sendBuffer();
        xSemaphoreGive(i2c_mutex);
    }
}
