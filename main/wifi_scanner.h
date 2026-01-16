/**
 * @file wifi_scanner.h
 * @brief WiFi scanner background task - periodically scans for networks using non-blocking scan
 */

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Default scan interval in seconds */
#define WIFI_SCANNER_DEFAULT_INTERVAL_SEC   30

/** Maximum number of APs to store from a single scan */
#define WIFI_SCANNER_MAX_AP_COUNT           20

/**
 * @brief Initialize and start the WiFi scanner background task
 * 
 * This function creates a low-priority FreeRTOS task that periodically
 * scans for WiFi networks using non-blocking scan and logs the results.
 * 
 * @note WiFi must be initialized and started in STA mode before calling this function
 * 
 * @return ESP_OK on success, or an error code on failure
 */
esp_err_t wifi_scanner_init(void);

/**
 * @brief Set the interval between WiFi scans
 * 
 * @param seconds Interval in seconds between scans (minimum 5 seconds)
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if seconds < 5
 */
esp_err_t wifi_scanner_set_interval(uint32_t seconds);

/**
 * @brief Get the current scan interval
 * 
 * @return Current scan interval in seconds
 */
uint32_t wifi_scanner_get_interval(void);

/**
 * @brief Stop the WiFi scanner task
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_scanner_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_SCANNER_H */
