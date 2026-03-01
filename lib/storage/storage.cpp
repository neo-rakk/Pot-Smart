#include "storage.h"
#include <Preferences.h>
#include <SPIFFS.h>
#include <nvs_flash.h>
#include "esp_err.h"

static Preferences prefs;

void storage_init(void) {
    // Arduino-style NVS
    // Initialize the underlying NVS partition first to avoid low-level
    // errors like "nvs_open failed: NOT_FOUND" that are printed by the
    // Preferences internals on fresh devices.
    esp_err_t nvs_ret = nvs_flash_init();
    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Partition needs erasing/reformat
        nvs_flash_erase();
        nvs_ret = nvs_flash_init();
    }
    if (nvs_ret != ESP_OK) {
        Serial.printf("Warning: nvs_flash_init failed: %d\n", nvs_ret);
    }

    // Try to open the NVS namespace. If it fails, continue with defaults but
    // avoid calling prefs.getBytes which will print lower-level error messages.
    bool prefs_ok = prefs.begin("plant_cfg", false);

    plant_config_init();

    // If NVS opened successfully, try to load saved config. If none exists
    // yet, persist the default config so subsequent boots don't log NOT_FOUND.
    if (prefs_ok) {
        plant_config_t loaded_cfg;
        size_t read = prefs.getBytes("config", &loaded_cfg, sizeof(plant_config_t));
        if (read == sizeof(plant_config_t)) {
            update_plant_config(loaded_cfg.plant_name, loaded_cfg.current_min_humidity, loaded_cfg.current_max_humidity, loaded_cfg.auto_mode);
        } else {
            // No saved config yet — save current defaults so future boots are silent.
            plant_config_t defaults = get_plant_config();
            prefs.putBytes("config", &defaults, sizeof(plant_config_t));
        }
    } else {
        Serial.println("Warning: NVS (Preferences) not available — using defaults");
    }

    // SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }
}

void save_config_to_nvs(const plant_config_t *cfg) {
    if (cfg) {
        prefs.putBytes("config", cfg, sizeof(plant_config_t));
    }
}
