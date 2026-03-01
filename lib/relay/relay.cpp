#include "relay.h"

static uint8_t relay_pin = 255;
static bool relay_state = false;
static TimerHandle_t relay_timer = NULL;

#define RELAY_MAX_ON_TIME_MS 10000

static void relay_safety_callback(TimerHandle_t xTimer) {
    relay_off();
}

void relay_init(uint8_t pin) {
    relay_pin = pin;
    pinMode(relay_pin, OUTPUT);
    digitalWrite(relay_pin, LOW);
    relay_state = false;
    relay_timer = xTimerCreate("RelaySafety", pdMS_TO_TICKS(RELAY_MAX_ON_TIME_MS), pdFALSE, (void*)0, relay_safety_callback);
}

void relay_on(void) {
    if (relay_pin != 255) {
        digitalWrite(relay_pin, HIGH);
        relay_state = true;
        if (relay_timer) xTimerStart(relay_timer, 0);
    }
}

void relay_off(void) {
    if (relay_pin != 255) {
        digitalWrite(relay_pin, LOW);
        relay_state = false;
        if (relay_timer) xTimerStop(relay_timer, 0);
    }
}

bool relay_is_on(void) {
    return relay_state;
}
