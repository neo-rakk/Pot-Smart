#ifndef ARROSAGE_H
#define ARROSAGE_H

#include "project_config.h"

void arrosage_init(void);
void arrosage_task(void *pvParameters);
logic_data_t arrosage_get_logic_data(void);

#endif
