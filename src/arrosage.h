#ifndef ARROSAGE_H
#define ARROSAGE_H

/**
 * @brief FreeRTOS task responsible for automatic watering logic.
 *
 * Monitors sensor data via a queue and controls the relay based on
 * moisture thresholds defined in the plant configuration.
 */
void arrosage_task(void *pvParameters);

#endif
