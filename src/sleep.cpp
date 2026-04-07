/**
 * @file sleep.cpp
 * @brief Sleep Implementation
 *
 * EXTRACTED FROM: TRMNL-T5S3-Pro/src/main.cpp (lines 713-731)
 * Phase 5: Updated to use button module for wakeup detection
 * Phase 5 Fix v4: Using GPIO0 (boot button) for EXT0 deep sleep wakeup
 *                 Matches official LilyGo firmware implementation
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_rom_gpio.h>
#include "sleep.h"
#include "config.h"
#include "button.h"

// PCA9555 is a C library, need extern "C" for C++ to link properly
extern "C" {
#include "board/pca9555.h"
}

void enter_deep_sleep(uint16_t sleep_seconds) {
    // Use parameter if provided, otherwise use config default
    if (sleep_seconds == 0) {
        sleep_seconds = TRMNL_DEEP_SLEEP_SECONDS;
    }

    Serial.println("[SLEEP] Entering deep sleep...");
    Serial.print("[SLEEP] Timer wakeup in: ");
    Serial.print(sleep_seconds);
    Serial.println(" seconds");
    Serial.println("[SLEEP] Button press will also wake device");
    delay(1000);

    // Phase 5 Fix v4: Suspend button polling task before deep sleep
    // This allows EXT0 interrupt to wake the device without task interference
    button_suspend();
    delay(100);

    // Phase 5 Fix v4: Configure GPIO0 (boot button) for EXT0 wakeup
    // GPIO0 is RTC-capable and officially supports EXT0 on ESP32-S3
    // This matches the official LilyGo firmware implementation
    Serial.println("[SLEEP] Configuring GPIO0 (boot button) for EXT0 wakeup...");

    // Reset GPIO0 to clear any previous configurations
    gpio_reset_pin(GPIO_NUM_0);

    // Set GPIO0 as input
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);

    // Enable internal pull-up (button pulls LOW when pressed)
    gpio_pullup_en(GPIO_NUM_0);

    // Keep RTC GPIO power domain powered during sleep
    // This is CRITICAL - without this, GPIO0 cannot trigger EXT0 wakeup
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    Serial.println("[SLEEP] GPIO0 configured for EXT0 wakeup");

    // Enable button wakeup on GPIO0 using EXT0 (triggers on LOW)
    // This is the official LilyGo approach for button wakeup from deep sleep
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // 0 = wake on LOW

    // Enable timer wakeup as fallback (device always wakes after this period)
    esp_sleep_enable_timer_wakeup(sleep_seconds * 1000000ULL);

    Serial.println("[SLEEP] Device ready for wakeup via button or timer");
    delay(100);

    esp_deep_sleep_start();  // Does not return
}
