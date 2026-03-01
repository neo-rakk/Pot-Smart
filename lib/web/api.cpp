#include "api.h"
#include "plant_config.h"
#include "sensors.h"
#include "relay.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>
#include <WebServer.h>

extern WebServer server;

static void status_handler() {
    JsonDocument doc;
    sensor_data_t data = sensors_get_latest_data();
    plant_config_t cfg = get_plant_config();

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
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        update_plant_config(doc["plant_name"], doc["min_humidity"], doc["max_humidity"], doc["auto_mode"]);
        server.send(200, "text/plain", "OK");
    }
}

static void wifi_handler() {
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        wifi_save_sta_config(doc["ssid"], doc["pass"]);
        server.send(200, "text/plain", "OK. Restarting...");
        delay(500);
        ESP.restart();
    }
}

static void valve_handler() {
    if (server.hasArg("plain")) {
        JsonDocument doc;
        deserializeJson(doc, server.arg("plain"));
        String act = doc["action"];
        if (act == "open") relay_on();
        else if (act == "close") relay_off();
        server.send(200, "text/plain", "OK");
    }
}

static void profiles_handler() {
    JsonDocument doc;
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
