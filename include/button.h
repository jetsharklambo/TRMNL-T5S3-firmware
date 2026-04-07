/**
 * @file button.h
 * @brief Button detection with active polling and hold detection
 *
 * Phase 5: Enhanced button handling with:
 * - Active polling task (300ms interval) for real-time button detection
 * - Button hold duration tracking (5s soft reset, 15s hard reset)
 * - Wake-up detection after deep sleep
 *
 * The device can detect button presses in two scenarios:
 * 1. During normal operation: Via polling task (detects all presses)
 * 2. During deep sleep: Via GPIO38 EXT0 interrupt wakeup
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>
#include <driver/gpio.h>

// ============================================================================
// Button Configuration
// ============================================================================

/**
 * @brief GPIO pin for button polling during normal operation
 *
 * The LilyGo T5S3 Pro button is connected to the PCA9555 I/O expander (I2C address 0x20).
 * When button is pressed, the PCA9555 pulls GPIO38 LOW.
 * GPIO38 is used for active polling during normal operation (every 300ms).
 *
 * For deep sleep wakeup, we use GPIO0 (boot button) via EXT0, which matches the
 * official LilyGo firmware implementation.
 */
#define BUTTON_WAKEUP_PIN GPIO_NUM_38  // PCA9555 interrupt line (for polling)

/**
 * @brief Button hold detection thresholds (milliseconds)
 */
#define BUTTON_HOLD_SOFT_RESET_MS 5000    // 5 seconds: soft reset (reload config, reboot)
#define BUTTON_HOLD_HARD_RESET_MS 15000   // 15 seconds: hard reset (clear credentials, enter AP mode)

/**
 * @brief Polling interval for button detection task
 */
#define BUTTON_POLL_INTERVAL_MS 300       // Check button every 300ms (like official firmware)

/**
 * @brief Debounce time to filter noise
 */
#define BUTTON_DEBOUNCE_MS 50             // Ignore rapid state changes

// ============================================================================
// Button Event Types
// ============================================================================

typedef enum {
    BUTTON_EVENT_PRESSED,      // Button just pressed
    BUTTON_EVENT_RELEASED,     // Button just released
    BUTTON_EVENT_HELD_5S,      // Button held for 5 seconds (soft reset)
    BUTTON_EVENT_HELD_15S      // Button held for 15 seconds (hard reset)
} button_event_t;

/**
 * @brief Callback function type for button events
 * @param event The button event that occurred
 * @param hold_duration_ms How long button has been held (in milliseconds)
 */
typedef void (*button_callback_t)(button_event_t event, uint32_t hold_duration_ms);

// ============================================================================
// Public Interface
// ============================================================================

/**
 * @brief Initialize button detection system
 *
 * Creates the button polling task that runs every 300ms.
 * Must be called in setup() before main app loop.
 *
 * @return true if initialization successful
 * @return false if initialization failed
 */
bool button_init();

/**
 * @brief Register callback for button events
 *
 * The callback will be invoked when button events occur:
 * - BUTTON_EVENT_PRESSED: Button just pressed
 * - BUTTON_EVENT_RELEASED: Button just released
 * - BUTTON_EVENT_HELD_5S: Button held 5 seconds (soft reset trigger)
 * - BUTTON_EVENT_HELD_15S: Button held 15 seconds (hard reset trigger)
 *
 * @param callback Function to call on button events
 */
void button_register_callback(button_callback_t callback);

/**
 * @brief Check if button is currently pressed
 *
 * @return true if button is held down
 * @return false if button is released
 */
bool button_is_pressed();

/**
 * @brief Get current button hold duration (if pressed)
 *
 * @return Milliseconds button has been held, or 0 if not pressed
 */
uint32_t button_get_hold_duration_ms();

/**
 * @brief Check if device was woken by button press from deep sleep
 *
 * @return true if woken by GPIO0 (boot button) via EXT0 interrupt (user action)
 * @return false if woken by timer (scheduled update)
 *
 * During deep sleep, GPIO0 (boot button) is configured as the EXT0 wakeup source.
 * This is separate from the GPIO38 polling mechanism used during normal operation.
 * This is used at boot to determine why device woke up.
 */
bool button_was_pressed();

/**
 * @brief Get numeric wakeup reason code
 *
 * @return uint32_t ESP32 wakeup cause code
 *
 * Returns the raw ESP32 wakeup cause for debugging.
 * Possible values:
 * - ESP_SLEEP_WAKEUP_EXT0 (1): Button press
 * - ESP_SLEEP_WAKEUP_TIMER (2): Timer wakeup
 * - Other values: Boot or other wakeup sources
 */
uint32_t button_get_wakeup_cause();

/**
 * @brief Suspend button polling task before deep sleep
 *
 * Stops the FreeRTOS polling task so it doesn't interfere with
 * EXT0 interrupt-based wakeup. Must be called before entering
 * deep sleep to allow button press to wake device.
 *
 * Call button_resume() after waking from sleep to re-enable polling.
 */
void button_suspend();

/**
 * @brief Resume button polling task after waking from sleep
 *
 * Re-enables the FreeRTOS polling task after waking from deep sleep.
 * Should be called immediately after app_main() starts following a wakeup.
 */
void button_resume();

#endif // BUTTON_H
