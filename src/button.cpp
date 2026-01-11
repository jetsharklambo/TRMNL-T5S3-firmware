/**
 * @file button.cpp
 * @brief Button detection with active polling and hold detection
 *
 * Phase 5: Implements button polling task that runs every 300ms to detect
 * button presses during normal operation. Also detects wake reason after
 * deep sleep via EXT0 interrupt.
 *
 * Architecture:
 * - FreeRTOS task polls GPIO38 every 300ms
 * - Tracks button press state and hold duration
 * - Invokes callbacks on press/release/hold milestones
 * - Debounces rapid state changes (50ms)
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "button.h"

extern "C" {
#include "board/pca9555.h"
}

// ============================================================================
// Global State
// ============================================================================

static bool button_task_running = false;
static bool button_currently_pressed = false;
static uint32_t button_press_start_time = 0;
static uint32_t last_state_change_time = 0;
static button_callback_t button_callback = nullptr;
static TaskHandle_t button_task_handle = nullptr;

// Flags to track which hold events have been fired
static bool soft_reset_event_fired = false;
static bool hard_reset_event_fired = false;

// Boot stabilization tracking
static bool boot_stabilization_done = false;
static bool boot_button_was_pressed = false;

// ============================================================================
// Button Reading Function (I2C)
// ============================================================================

/**
 * @brief Read button state from PCA9555 I/O expander via I2C
 *
 * The button is on PCA9555 port 1, pin PC12.
 * Returns true if button is pressed (active LOW on GPIO38).
 */
static bool button_read_from_pca9555() {
    // Read the interrupt pin state directly
    int pin_state = digitalRead(BUTTON_WAKEUP_PIN);

    // GPIO38 is pulled LOW by PCA9555 when button is pressed
    return (pin_state == LOW);
}

// ============================================================================
// FreeRTOS Task
// ============================================================================

/**
 * @brief Button polling task (runs every 300ms)
 *
 * Polls GPIO38 to detect button state changes.
 * Tracks hold duration and invokes callbacks.
 */
static void button_polling_task(void *param) {
    Serial.println("[BUTTON] Polling task started");

    // Phase 1: Boot stabilization - wait for hardware to settle
    Serial.println("[BUTTON] Waiting 3 seconds for hardware stabilization...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Phase 2: Read initial button state and store it
    boot_button_was_pressed = button_read_from_pca9555();
    if (boot_button_was_pressed) {
        Serial.println("[BUTTON] Button detected as pressed at boot - will ignore until released");
    } else {
        Serial.println("[BUTTON] Button not pressed at boot");
    }

    boot_stabilization_done = true;
    Serial.println("[BUTTON] Hardware stabilized, monitoring button state changes");

    while (1) {
        vTaskDelay(BUTTON_POLL_INTERVAL_MS / portTICK_PERIOD_MS);

        // Read current button state
        bool button_now_pressed = button_read_from_pca9555();
        uint32_t now = millis();

        // Debounce: ignore changes that happen too quickly
        if (now - last_state_change_time < BUTTON_DEBOUNCE_MS) {
            continue;
        }

        // ====== BUTTON PRESSED ======
        if (button_now_pressed && !button_currently_pressed) {
            // Ignore first press if button was held during boot
            if (boot_button_was_pressed && boot_stabilization_done) {
                Serial.println("[BUTTON] Ignoring initial boot press - waiting for release first");
                boot_button_was_pressed = false;  // Clear flag after first ignore
                continue;
            }

            // Transition: released → pressed
            button_currently_pressed = true;
            button_press_start_time = now;
            last_state_change_time = now;

            // Reset hold event flags
            soft_reset_event_fired = false;
            hard_reset_event_fired = false;

            Serial.println("[BUTTON] PRESSED");
            if (button_callback) {
                button_callback(BUTTON_EVENT_PRESSED, 0);
            }
        }

        // ====== BUTTON RELEASED ======
        else if (!button_now_pressed && button_currently_pressed) {
            // Transition: pressed → released
            uint32_t hold_duration = now - button_press_start_time;
            button_currently_pressed = false;
            last_state_change_time = now;

            Serial.print("[BUTTON] RELEASED (held ");
            Serial.print(hold_duration);
            Serial.println("ms)");

            if (button_callback) {
                button_callback(BUTTON_EVENT_RELEASED, hold_duration);
            }
        }
        // ====== BUTTON RELEASED (from boot press) ======
        else if (!button_now_pressed && boot_button_was_pressed) {
            // Button was pressed at boot and just released - clear the flag
            Serial.println("[BUTTON] Boot press released - ready for normal operation");
            boot_button_was_pressed = false;
            last_state_change_time = now;
        }

        // ====== BUTTON HELD - 5 SECOND MARK ======
        else if (button_currently_pressed && !soft_reset_event_fired) {
            uint32_t hold_duration = now - button_press_start_time;

            if (hold_duration >= BUTTON_HOLD_SOFT_RESET_MS) {
                soft_reset_event_fired = true;
                Serial.print("[BUTTON] HELD 5 seconds (");
                Serial.print(hold_duration);
                Serial.println("ms) - triggering soft reset");

                if (button_callback) {
                    button_callback(BUTTON_EVENT_HELD_5S, hold_duration);
                }
            }
        }

        // ====== BUTTON HELD - 15 SECOND MARK ======
        else if (button_currently_pressed && soft_reset_event_fired && !hard_reset_event_fired) {
            uint32_t hold_duration = now - button_press_start_time;

            if (hold_duration >= BUTTON_HOLD_HARD_RESET_MS) {
                hard_reset_event_fired = true;
                Serial.print("[BUTTON] HELD 15 seconds (");
                Serial.print(hold_duration);
                Serial.println("ms) - triggering hard reset");

                if (button_callback) {
                    button_callback(BUTTON_EVENT_HELD_15S, hold_duration);
                }
            }
        }
    }
}

// ============================================================================
// Public Interface
// ============================================================================

bool button_init() {
    Serial.println("[BUTTON] Initializing button detection...");

    // Ensure GPIO38 is configured as input with pull-up
    // This pin is connected to the PCA9555 interrupt line
    pinMode(BUTTON_WAKEUP_PIN, INPUT_PULLUP);

    // Small delay to let the pin settle
    delay(100);

    // Create FreeRTOS task for button polling
    // Stack size: 3KB (same as official firmware's btn_task)
    // Priority: 10 (high priority, important for responsiveness)
    BaseType_t task_created = xTaskCreate(
        button_polling_task,
        "button_task",
        1024 * 3,              // 3KB stack
        nullptr,               // parameter
        10,                    // priority (high)
        &button_task_handle    // task handle (for suspend/resume)
    );

    if (task_created == pdPASS) {
        button_task_running = true;
        Serial.println("[BUTTON] Polling task created successfully");
        return true;
    } else {
        Serial.println("[BUTTON] ERROR: Failed to create polling task!");
        return false;
    }
}

void button_register_callback(button_callback_t callback) {
    button_callback = callback;
    if (callback) {
        Serial.println("[BUTTON] Callback registered");
    }
}

bool button_is_pressed() {
    return button_currently_pressed;
}

uint32_t button_get_hold_duration_ms() {
    if (!button_currently_pressed) {
        return 0;
    }
    return millis() - button_press_start_time;
}

bool button_was_pressed() {
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    // GPIO0 (boot button) uses EXT0 wakeup from deep sleep
    // Matches official LilyGo firmware implementation
    return wakeup_cause == ESP_SLEEP_WAKEUP_EXT0;
}

uint32_t button_get_wakeup_cause() {
    return (uint32_t)esp_sleep_get_wakeup_cause();
}

void button_suspend() {
    if (button_task_handle != nullptr) {
        Serial.println("[BUTTON] Suspending polling task before deep sleep");
        vTaskSuspend(button_task_handle);
    }
}

void button_resume() {
    if (button_task_handle != nullptr) {
        Serial.println("[BUTTON] Resuming polling task after waking");
        vTaskResume(button_task_handle);
    }
}
