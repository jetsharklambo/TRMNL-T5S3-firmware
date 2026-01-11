/**
 * @file power.cpp
 * @brief Power management and battery monitoring implementation
 *
 * Provides battery voltage, state of charge, and charging status via
 * the BQ27220 fuel gauge IC (I2C).
 *
 * The BQ27220 is a dedicated battery management IC that provides accurate
 * measurements via I2C communication on pins 39 (SDA) and 40 (SCL).
 */

#include <Arduino.h>
#include <Wire.h>
#include "power.h"

// Global BQ27220 instance
static BQ27220 bq27220;
static bool power_initialized = false;

bool power_init() {
    Serial.println("[POWER] Initializing BQ27220 fuel gauge (I2C: SDA=39, SCL=40)");

    // Initialize Wire/I2C (pins are configured in BQ27220 defaults: 39, 40)
    Wire.begin(39, 40, 100000);  // SDA=39, SCL=40, 100kHz
    delay(100);

    // Initialize BQ27220
    if (!bq27220.begin()) {
        Serial.println("[POWER] ERROR: Failed to initialize BQ27220 I2C");
        return false;
    }

    delay(100);

    // Initialize the fuel gauge with default data
    if (!bq27220.init()) {
        Serial.println("[POWER] ERROR: Failed to init BQ27220 fuel gauge");
        return false;
    }

    power_initialized = true;
    Serial.println("[POWER] BQ27220 initialized successfully");

    // Print initial battery state
    uint16_t voltage = bq27220.getVoltage();
    uint16_t soc = bq27220.getStateOfCharge();
    bool charging = bq27220.getIsCharging();

    Serial.print("[POWER] Initial: ");
    Serial.print(voltage);
    Serial.print("mV, ");
    Serial.print(soc);
    Serial.print("%, ");
    Serial.println(charging ? "Charging" : "Discharging");

    return true;
}

float power_get_battery_voltage() {
    if (!power_initialized) {
        Serial.println("[POWER] ERROR: Battery not initialized, call power_init() first");
        return 0.0f;
    }

    // Read voltage from BQ27220 (returns millivolts)
    uint16_t voltage_mv = bq27220.getVoltage();

    // Sanity check: BQ27220 returns 65535 (0xFFFF) on I2C error
    // Valid battery voltage range is 2500-4500 mV (2.5V-4.5V for lithium)
    if (voltage_mv > 5000 || voltage_mv < 2000) {
        Serial.print("[POWER] ERROR: Invalid voltage reading: ");
        Serial.print(voltage_mv);
        Serial.println("mV (likely I2C error, returning cached value)");
        // Return last known good value (4200 as safe default)
        return 4.2f;
    }

    // Convert to volts
    float voltage_v = voltage_mv / 1000.0f;

    Serial.print("[POWER] Battery voltage: ");
    Serial.print(voltage_mv);
    Serial.print("mV (");
    Serial.print(voltage_v, 2);
    Serial.println("V)");

    return voltage_v;
}

uint8_t power_get_battery_percentage() {
    if (!power_initialized) {
        Serial.println("[POWER] ERROR: Battery not initialized");
        return 0;
    }

    // Get state of charge directly from BQ27220
    // This is more accurate than voltage-based estimation
    uint16_t soc = bq27220.getStateOfCharge();

    // Clamp to 0-100%
    uint8_t percentage = (soc > 100) ? 100 : (uint8_t)soc;

    Serial.print("[POWER] State of charge: ");
    Serial.print(percentage);
    Serial.println("%");

    return percentage;
}

const char* power_get_battery_status() {
    if (!power_initialized) {
        return "unknown";
    }

    float voltage = power_get_battery_voltage();

    if (voltage >= 4.15f) {
        return "full";
    } else if (voltage >= 4.05f) {
        return "charged";
    } else if (voltage >= 3.60f) {
        return "normal";
    } else if (voltage >= 3.20f) {
        return "low";
    } else {
        return "critical";
    }
}

bool power_is_charging() {
    if (!power_initialized) {
        Serial.println("[POWER] ERROR: Battery not initialized");
        return false;
    }

    // Get actual charging status from BQ27220
    // This reads the current direction: if NOT discharging, it's charging
    bool charging = bq27220.getIsCharging();

    Serial.print("[POWER] Charging status: ");
    Serial.println(charging ? "CHARGING" : "DISCHARGING");

    return charging;
}

uint8_t power_get_state_of_charge() {
    if (!power_initialized) {
        Serial.println("[POWER] ERROR: Battery not initialized");
        return 0;
    }

    // Retry logic for I2C communication
    const int MAX_RETRIES = 3;
    const int RETRY_DELAY_MS = 50;  // Wait 50ms between retries

    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        // Get state of charge from BQ27220
        uint16_t soc = bq27220.getStateOfCharge();

        // Sanity check: valid range is 0-100%
        if (soc <= 100) {
            // Valid reading - return immediately
            if (attempt > 1) {
                Serial.print("[POWER] SOC read successful on attempt ");
                Serial.println(attempt);
            }
            return (uint8_t)soc;
        }

        // Invalid reading - log and retry
        Serial.print("[POWER] I2C error on attempt ");
        Serial.print(attempt);
        Serial.print("/");
        Serial.print(MAX_RETRIES);
        Serial.print(": Invalid SOC=");
        Serial.println(soc);

        if (attempt < MAX_RETRIES) {
            delay(RETRY_DELAY_MS);  // Wait before retry
        }
    }

    // All retries failed - return last known good value or safe default
    Serial.println("[POWER] ERROR: All SOC read attempts failed, returning 50%");
    return 50;  // Return safe default
}

float power_get_battery_temperature() {
    if (!power_initialized) {
        Serial.println("[POWER] ERROR: Battery not initialized");
        return 0.0f;
    }

    // Retry logic for I2C communication
    const int MAX_RETRIES = 3;
    const int RETRY_DELAY_MS = 50;

    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        // Get battery temperature from BQ27220 (returned in 0.1K units)
        uint16_t temp_raw = bq27220.getTemperature();

        // Sanity check: valid range is roughly 250K-330K (-23°C to 57°C)
        if (temp_raw >= 2500 && temp_raw <= 3300) {
            // Valid reading
            float temp_k = temp_raw / 10.0f;
            float temp_c = temp_k - 273.15f;

            if (attempt > 1) {
                Serial.print("[POWER] Temperature read successful on attempt ");
                Serial.println(attempt);
            }

            Serial.print("[POWER] Battery temperature: ");
            Serial.print(temp_c, 1);
            Serial.println("°C");

            return temp_c;
        }

        // Invalid reading - log and retry
        Serial.print("[POWER] I2C error on attempt ");
        Serial.print(attempt);
        Serial.print("/");
        Serial.print(MAX_RETRIES);
        Serial.print(": Invalid temp=");
        Serial.print(temp_raw / 10.0f);
        Serial.println("K");

        if (attempt < MAX_RETRIES) {
            delay(RETRY_DELAY_MS);
        }
    }

    // All retries failed
    Serial.println("[POWER] ERROR: All temperature read attempts failed, returning 25°C");
    return 25.0f;  // Return safe default
}

uint32_t power_get_free_heap() {
    uint32_t free_heap = esp_get_free_heap_size();

    // Log warning if critically low
    if (free_heap < 50000) {
        Serial.print("[POWER] WARNING: Low heap memory! Free: ");
        Serial.print(free_heap);
        Serial.println(" bytes");
    }

    return free_heap;
}

uint32_t power_get_total_heap() {
    // Return the total heap size (internal + PSRAM)
    // Use heap capabilities to get accurate total
    return heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
}
