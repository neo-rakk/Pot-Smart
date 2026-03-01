#ifndef STORAGE_H
#define STORAGE_H

#include "plant_config.h"

void storage_init(void);
void save_config_to_nvs(const plant_config_t *cfg);

#endif
