/**
 * @file device_status.cpp
 * @brief Device status collection implementation (Phase 6)
 *
 * Gathers telemetry from all subsystems for logging context
 */

#include "device_status.h"
#include "power.h"
#include <WiFi.h>
#include <Arduino.h>

// ============================================================================
// Static Storage for Status Information
// ============================================================================

static char g_current_image[64] = "unknown";
static char g_last_error[128] = "none";
static uint32_t g_boot_time_ms = 0;

// ============================================================================
// Public Interface Implementation
// ============================================================================

device_status_t status_get_current() {
    device_status_t status = {};

    // Battery telemetry
    status.battery_voltage = power_get_battery_voltage();
    status.battery_soc = power_get_state_of_charge();
    status.battery_temp = power_get_battery_temperature();

    // WiFi signal strength
    status.wifi_rssi = WiFi.RSSI();

    // Memory usage
    status.free_heap = power_get_free_heap();
    status.total_heap = power_get_total_heap();

    // Stored context
    snprintf(status.current_image, sizeof(status.current_image), "%s", g_current_image);
    snprintf(status.last_error, sizeof(status.last_error), "%s", g_last_error);

    // Device uptime
    if (g_boot_time_ms == 0) {
        g_boot_time_ms = millis();
    }
    status.uptime_seconds = (millis() - g_boot_time_ms) / 1000;

    return status;
}

void status_set_current_image(const char* filename) {
    if (filename) {
        snprintf(g_current_image, sizeof(g_current_image), "%s", filename);
        Serial.print("[STATUS] Set current image: ");
        Serial.println(g_current_image);
    }
}

void status_set_last_error(const char* error) {
    if (error) {
        snprintf(g_last_error, sizeof(g_last_error), "%s", error);
        Serial.print("[STATUS] Set last error: ");
        Serial.println(g_last_error);
    }
}

float status_get_battery_voltage() {
    return power_get_battery_voltage();
}

uint8_t status_get_battery_soc() {
    return power_get_state_of_charge();
}

int status_get_wifi_rssi() {
    int rssi = WiFi.RSSI();
    // WiFi.RSSI() returns 0 if not connected, return reasonable default
    return (rssi == 0) ? -100 : rssi;
}

uint32_t status_get_free_heap() {
    return power_get_free_heap();
}

uint32_t status_get_total_heap() {
    return power_get_total_heap();
}
