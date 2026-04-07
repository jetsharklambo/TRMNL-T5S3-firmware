/**
 * @file sleep.h
 * @brief Deep sleep and wakeup functions
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 713-731)
 * Phase 4: Added configurable sleep duration for dynamic refresh rates
 */

#ifndef SLEEP_H
#define SLEEP_H

#include <cstdint>

/**
 * @brief Configure wakeup sources and enter deep sleep
 *
 * Enables two wakeup sources:
 * 1. GPIO38 button press via PCA9555 (wakeup on LOW)
 * 2. Timer (configurable duration)
 *
 * Whichever occurs first will wake the device.
 * On wakeup, device reboots and calls app_main() again.
 *
 * Power consumption during sleep:
 * - Without GPIO activity: ~4 µA
 * - With GPIO38 monitoring: ~30-100 µA average
 *
 * CRITICAL: WiFi must be disconnected before calling this function.
 * Call wifi_disconnect_and_poweroff() first.
 *
 * @param sleep_seconds Sleep duration in seconds. If 0 or not provided,
 *                     uses TRMNL_DEEP_SLEEP_SECONDS from config.h
 *
 * NOTE: This function does not return (device resets on wakeup)
 */
void enter_deep_sleep(uint16_t sleep_seconds = 0);

#endif // SLEEP_H
