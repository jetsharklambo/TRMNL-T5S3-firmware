/**
 * @file device_status.h
 * @brief Device status collection and telemetry (Phase 6)
 *
 * Collects device status information from all subsystems:
 * - Battery voltage, SOC, temperature
 * - WiFi signal strength
 * - Memory usage
 * - Current image
 * - Last error
 */

#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

#include "log_entry.h"

/**
 * @brief Collect current device status
 *
 * Gathers telemetry from:
 * - BQ27220 fuel gauge (battery)
 * - WiFi stack (RSSI)
 * - ESP32 heap
 * - Persistent storage (current image)
 *
 * @return device_status_t with current status
 */
device_status_t status_get_current();

/**
 * @brief Set current image filename
 *
 * Stores filename of currently displayed image
 * Used in logging for context
 *
 * @param filename Image filename (e.g., "2024-09-20T00:00:00")
 */
void status_set_current_image(const char* filename);

/**
 * @brief Set last error message
 *
 * Stores last error encountered
 * Used in logging for diagnostics
 *
 * @param error Error message
 */
void status_set_last_error(const char* error);

/**
 * @brief Get current battery voltage
 *
 * @return Battery voltage in volts (e.g., 4.2)
 */
float status_get_battery_voltage();

/**
 * @brief Get current battery SOC
 *
 * @return Battery state of charge 0-100%
 */
uint8_t status_get_battery_soc();

/**
 * @brief Get WiFi signal strength
 *
 * @return RSSI in dBm (typically -30 to -100)
 */
int status_get_wifi_rssi();

/**
 * @brief Get free heap memory
 *
 * @return Free heap in bytes
 */
uint32_t status_get_free_heap();

/**
 * @brief Get total heap memory
 *
 * @return Total heap in bytes
 */
uint32_t status_get_total_heap();

#endif // DEVICE_STATUS_H
