/**
 * @file submit_log.h
 * @brief TRMNL API /api/log endpoint client (Phase 6)
 *
 * Submits device logs to TRMNL server for diagnostics and monitoring
 */

#ifndef SUBMIT_LOG_H
#define SUBMIT_LOG_H

#include "logging.h"

/**
 * @brief Submit a single log entry to /api/log
 *
 * Sends log entry to server in JSON format:
 * ```json
 * {
 *   "device_id": "917F0B",
 *   "timestamp": 1695206400,
 *   "log_message": "WiFi connection failed",
 *   "device_status": {
 *     "wifi_rssi": -65,
 *     "battery_voltage": 3.8,
 *     "battery_soc": 45,
 *     "free_heap": 125000,
 *     "current_image": "2024-09-20T00:00:00",
 *     "current_error": "Download timeout"
 *   }
 * }
 * ```
 *
 * @param api_key TRMNL API key for authentication
 * @param device_id TRMNL device ID (friendly ID, e.g., "917F0B")
 * @param entry Log entry to submit
 * @return true if submission successful
 * @return false if network error or server error
 */
bool trmnlApiSubmitLog(const char* api_key, const char* device_id, const log_entry_t* entry);

/**
 * @brief Submit multiple log entries to /api/log
 *
 * More efficient than submitting individual entries
 * Batches logs in single HTTP request
 *
 * @param api_key TRMNL API key
 * @param device_id TRMNL device ID
 * @param entries Array of log entries
 * @param count Number of entries in array
 * @return true if submission successful
 * @return false if network error or server error
 */
bool trmnlApiSubmitLogBatch(const char* api_key, const char* device_id,
                            const log_entry_t* entries, uint32_t count);

#endif // SUBMIT_LOG_H
