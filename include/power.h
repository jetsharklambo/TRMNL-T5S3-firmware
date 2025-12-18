/**
 * @file power.h
 * @brief Power management and battery monitoring interface
 *
 * Provides functions to read battery voltage, state of charge, and charging status
 * via the BQ27220 fuel gauge IC (I2C on pins 39/40).
 *
 * The BQ27220 is a dedicated battery management IC that provides:
 * - Accurate voltage measurement
 * - Real charging/discharging current
 * - State of charge (battery percentage)
 * - Battery temperature
 * - Charging status detection
 * - State of health monitoring
 */

#ifndef POWER_H
#define POWER_H

#include <cstdint>
#include "bq27220.h"

/**
 * @brief Initialize power/battery monitoring via BQ27220
 *
 * Must be called once at startup before reading battery voltage.
 * Initializes I2C communication with BQ27220 fuel gauge IC (I2C pins: SDA=39, SCL=40).
 *
 * This function should be called early in the boot sequence,
 * before any API calls that need battery voltage data.
 *
 * @return true if initialization successful
 * @return false if BQ27220 initialization failed
 */
bool power_init();

/**
 * @brief Read current battery voltage from BQ27220
 *
 * @return float Battery voltage in volts (e.g., 3.8, 4.1)
 *
 * Reads battery voltage from BQ27220 fuel gauge IC via I2C.
 * Returns the actual battery cell voltage (accurate measurement).
 *
 * Typical values:
 * - 3.0V: Battery critically low
 * - 3.7V: Normal operation (mid-range capacity)
 * - 4.1V: Recently charged
 * - 4.2V: Fully charged (Lithium max)
 *
 * @return Battery voltage in volts (mV from IC / 1000.0)
 * @note Returns 0.0 if BQ27220 communication failed
 */
float power_get_battery_voltage();

/**
 * @brief Get free heap memory size
 *
 * @return uint32_t Free heap memory in bytes
 *
 * Returns the amount of free RAM available to the application.
 * Useful for detecting memory pressure or memory leaks.
 *
 * On ESP32-S3 with external PSRAM:
 * - Expected free heap after init: ~200-250 KB
 * - Warning threshold: < 50 KB (critical)
 * - Safe threshold: > 100 KB
 *
 * Usage:
 * @code
 * uint32_t free_heap = power_get_free_heap();
 * if (free_heap < 50000) {
 *     Serial.println("WARNING: Low memory!");
 * }
 * @endcode
 */
uint32_t power_get_free_heap();

/**
 * @brief Get total heap memory size
 *
 * @return uint32_t Total heap memory in bytes
 *
 * Returns the total available heap memory (used + free).
 */
uint32_t power_get_total_heap();

/**
 * @brief Get battery percentage based on voltage
 *
 * @return uint8_t Battery percentage (0-100)
 *
 * Converts raw battery voltage to a percentage using typical lithium
 * battery discharge curve. Useful for displaying to user or sending
 * to server for monitoring.
 *
 * Calculation uses linear interpolation:
 * - 4.20V = 100% (fully charged)
 * - 3.70V = 50% (nominal mid-point)
 * - 3.00V = 0% (critically low)
 *
 * @note Without hardware charge detection, this is voltage-based estimation only
 */
uint8_t power_get_battery_percentage();

/**
 * @brief Get human-readable battery status
 *
 * @return const char* Status string: "full", "charged", "normal", "low", or "critical"
 *
 * Returns battery status based on voltage thresholds:
 * - "full": >= 4.15V (at maximum charge)
 * - "charged": >= 4.05V (recently charged)
 * - "normal": >= 3.60V (normal operation range)
 * - "low": >= 3.20V (getting low, should recharge)
 * - "critical": < 3.20V (may shut down)
 *
 * @note This is estimated from voltage. Use power_is_charging() for actual charging status.
 */
const char* power_get_battery_status();

/**
 * @brief Check if battery is currently charging
 *
 * @return true if battery is actively charging, false if discharging
 *
 * Uses the BQ27220's internal charging detection.
 * The BQ27220 measures current direction and determines if battery
 * is actively receiving charge or being discharged.
 *
 * @note Returns false if BQ27220 communication failed
 * @note This is the REAL charging status, not estimated from voltage
 */
bool power_is_charging();

/**
 * @brief Get state of charge (battery percentage)
 *
 * @return uint8_t Battery percentage (0-100%)
 *
 * Returns the calculated state of charge from the BQ27220.
 * This is more accurate than voltage-based estimation because the
 * BQ27220 uses Coulomb counting (tracks current in/out) to estimate
 * how much capacity remains.
 *
 * - 0%: Battery empty
 * - 50%: Battery half-charged
 * - 100%: Battery fully charged
 *
 * @note Returns 0 if BQ27220 communication failed
 */
uint8_t power_get_state_of_charge();

/**
 * @brief Get battery temperature
 *
 * @return float Battery temperature in Celsius (e.g., 25.5)
 *
 * Reads the internal battery temperature from the BQ27220's
 * onboard temperature sensor.
 *
 * Typical ranges:
 * - -10°C to 0°C: Cold (charging may be disabled)
 * - 0°C to 45°C: Normal operating range
 * - 45°C to 60°C: Warm (may throttle charging)
 * - > 60°C: Critical (charging may be disabled)
 *
 * @note Returns 0.0 if BQ27220 communication failed
 */
float power_get_battery_temperature();

#endif // POWER_H
