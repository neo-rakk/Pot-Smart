#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

void relay_init(uint8_t pin);
void relay_on(void);
void relay_off(void);
bool relay_is_on(void);

#endif
