#include <unity.h>
#include <stdbool.h>

// Mocking needed for logic test
static bool relay_state = false;
void relay_on() { relay_state = true; }
void relay_off() { relay_state = false; }
bool relay_is_on() { return relay_state; }

typedef struct {
    int soil_moisture;
} sensor_data_t;

typedef struct {
    int min_humidity;
    int max_humidity;
    bool auto_mode;
} plant_config_t;

void process_watering_logic(sensor_data_t data, plant_config_t cfg) {
    if (cfg.auto_mode) {
        if (data.soil_moisture < cfg.min_humidity) {
            if (!relay_is_on()) relay_on();
        } else if (data.soil_moisture > cfg.max_humidity) {
            if (relay_is_on()) relay_off();
        }
    }
}

void test_watering_logic_should_turn_on_when_low() {
    relay_state = false;
    sensor_data_t data = { .soil_moisture = 20 };
    plant_config_t cfg = { .min_humidity = 30, .max_humidity = 60, .auto_mode = true };

    process_watering_logic(data, cfg);
    TEST_ASSERT_TRUE(relay_is_on());
}

void test_watering_logic_should_turn_off_when_high() {
    relay_state = true;
    sensor_data_t data = { .soil_moisture = 70 };
    plant_config_t cfg = { .min_humidity = 30, .max_humidity = 60, .auto_mode = true };

    process_watering_logic(data, cfg);
    TEST_ASSERT_FALSE(relay_is_on());
}

void test_watering_logic_should_do_nothing_when_in_range() {
    relay_state = false;
    sensor_data_t data = { .soil_moisture = 45 };
    plant_config_t cfg = { .min_humidity = 30, .max_humidity = 60, .auto_mode = true };

    process_watering_logic(data, cfg);
    TEST_ASSERT_FALSE(relay_is_on());

    relay_state = true;
    process_watering_logic(data, cfg);
    TEST_ASSERT_TRUE(relay_is_on());
}

void test_watering_logic_should_do_nothing_in_manual_mode() {
    relay_state = false;
    sensor_data_t data = { .soil_moisture = 10 };
    plant_config_t cfg = { .min_humidity = 30, .max_humidity = 60, .auto_mode = false };

    process_watering_logic(data, cfg);
    TEST_ASSERT_FALSE(relay_is_on());
}

void setup() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_watering_logic_should_turn_on_when_low);
    RUN_TEST(test_watering_logic_should_turn_off_when_high);
    RUN_TEST(test_watering_logic_should_do_nothing_when_in_range);
    RUN_TEST(test_watering_logic_should_do_nothing_in_manual_mode);
    return UNITY_END();
}
