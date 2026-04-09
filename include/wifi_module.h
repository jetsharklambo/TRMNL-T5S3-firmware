/**
 * @file wifi_module.h
 * @brief WiFi connectivity functions
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 639-658)
 * Simple wrapper around ESP32 WiFi library with retry logic
 */

#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

/**
 * @brief Initialize and connect to WiFi
 *
 * Uses hardcoded WIFI_SSID and WIFI_PASSWORD from wifi_config.h
 * Retries for up to 15 seconds
 *
 * @return true if connected, false if timeout (15 seconds)
 *
 * Time: 2-15 seconds depending on network
 * Power: 90-120 mA while connecting
 */
bool wifi_connect_blocking();

/**
 * @brief Disconnect from WiFi and power off radio
 *
 * CRITICAL for battery life - reduces power from 50-90 mA to <1 mA
 * Call this before deep sleep
 */
void wifi_disconnect_and_poweroff();

/**
 * @brief Get WiFi signal strength
 *
 * @return RSSI value in dBm (e.g., -65)
 * Negative value, closer to 0 is stronger signal
 */
int wifi_get_rssi();

/**
 * @brief Check if currently connected to WiFi
 *
 * @return true if connected, false otherwise
 */
bool wifi_is_connected();

/**
 * @brief Convert WiFi status code to human-readable string
 *
 * @param status WiFi status code from WiFi.status()
 * @return Readable string (e.g., "Connected", "No SSID Available")
 */
const char* wifi_status_string(wl_status_t status);

/**
 * @brief Print current WiFi status to Serial (debug helper)
 *
 * Only prints if DEBUG_WIFI_STATUS is enabled in test_config.h
 * Example output: "[WIFI_DEBUG] Status: Connected (3)"
 */
void debug_wifi_status();

#endif // WIFI_MODULE_H
