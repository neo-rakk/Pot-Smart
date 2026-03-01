#include "api.h"
#include "plant_config.h"
#include "sensors.h"
#include "relay.h"
#include "wifi_manager.h"
#include "arrosage.h"
#include <ArduinoJson.h>
#include <WebServer.h>

extern WebServer server;
extern String device_id;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

static void status_handler() {
    DynamicJsonDocument doc(1024);
    sensor_data_t data = sensors_get_latest_data();
    plant_config_t cfg = get_plant_config();
    logic_data_t logic = arrosage_get_logic_data();

    doc["device_id"] = device_id;
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["plant_profile"] = cfg.plant_name;

    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["soil"] = data.soil_moisture;
    sensors["air_temp"] = data.temperature;
    sensors["air_hum"] = data.humidity;
    sensors["co2"] = data.co2;
    sensors["tvoc"] = data.tvoc;

    JsonObject actuators = doc["actuators"].to<JsonObject>();
    actuators["pump_state"] = relay_is_on();
    actuators["auto_mode"] = cfg.auto_mode;

    JsonObject logic_obj = doc["logic"].to<JsonObject>();
    logic_obj["base_threshold_min"] = logic.base_threshold_min;
    logic_obj["calculated_threshold_min"] = logic.calculated_threshold_min;
    logic_obj["climate_mode"] = logic.climate_mode;

    // Backward compatibility or for index.html if not updated yet
    doc["soil_moisture"] = data.soil_moisture;
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["co2"] = data.co2;
    doc["tvoc"] = data.tvoc;
    doc["valve_is_on"] = relay_is_on();
    doc["plant_name"] = cfg.plant_name;
    doc["auto_mode"] = cfg.auto_mode;
    doc["min_humidity_cfg"] = cfg.current_min_humidity;
    doc["max_humidity_cfg"] = cfg.current_max_humidity;
    doc["sta_connected"] = wifi_is_sta_connected();

    String resp;
    serializeJson(doc, resp);
    server.send(200, "application/json", resp);
}

static void config_handler() {
    if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }
        update_plant_config(doc["plant_name"], doc["min_humidity"], doc["max_humidity"], doc["auto_mode"]);
        server.send(200, "text/plain", "OK");
    }
}

static void wifi_handler() {
    if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }
        const char* ssid = doc["ssid"] | "";
        const char* pass = doc["pass"] | "";
        bool ok = wifi_save_sta_config(ssid, pass);
        if (!ok) {
            server.send(500, "text/plain", "Failed to save/connect with provided WiFi credentials");
            return;
        }
        server.send(200, "text/plain", "OK. Restarting...");
        delay(500);
        ESP.restart();
    }
}

static void valve_handler() {
    if (server.hasArg("plain")) {
    DynamicJsonDocument doc(128);
        DeserializationError err = deserializeJson(doc, server.arg("plain"));
        if (err) {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }
        const char* act_c = doc["action"] | "";
        String act = String(act_c);
        if (act == "open") relay_on();
        else if (act == "close") relay_off();
        server.send(200, "text/plain", "OK");
    }
}

static void profiles_handler() {
    DynamicJsonDocument doc(512);
    int count = 0;
    const plant_profile_t *p = get_predefined_plants(&count);
    JsonArray arr = doc.to<JsonArray>();
    for(int i=0; i<count; i++) {
        JsonObject obj = arr.add<JsonObject>();
        obj["name"] = p[i].name;
        obj["default_min_humidity"] = p[i].default_min_humidity;
        obj["default_max_humidity"] = p[i].default_max_humidity;
    }
    String resp;
    serializeJson(doc, resp);
    server.send(200, "application/json", resp);
}

void register_api_handlers(void) {
    server.on("/api/status", HTTP_GET, status_handler);
    server.on("/api/config", HTTP_POST, config_handler);
    server.on("/api/wifi", HTTP_POST, wifi_handler);
    server.on("/api/valve", HTTP_POST, valve_handler);
    server.on("/api/plant_profiles", HTTP_GET, profiles_handler);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
