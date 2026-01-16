/**
 * @file wifi_scanner.c
 * @brief WiFi scanner background task implementation
 */

#include "wifi_scanner.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"

static const char *TAG = "wifi_scanner";

/** Event bit set when scan completes */
#define SCAN_DONE_BIT   BIT0

/** Scanner task stack size */
#define SCANNER_TASK_STACK_SIZE     4096

/** Scanner task priority (low priority background task) */
#define SCANNER_TASK_PRIORITY       (tskIDLE_PRIORITY + 1)

/** Minimum allowed scan interval in seconds */
#define MIN_SCAN_INTERVAL_SEC       5

/* Module state */
static EventGroupHandle_t s_scanner_event_group = NULL;
static TaskHandle_t s_scanner_task_handle = NULL;
static uint32_t s_scan_interval_sec = WIFI_SCANNER_DEFAULT_INTERVAL_SEC;
static volatile bool s_scanner_running = false;

/**
 * @brief Convert WiFi auth mode to string for logging
 */
static const char *auth_mode_to_str(wifi_auth_mode_t auth_mode)
{
    switch (auth_mode) {
        case WIFI_AUTH_OPEN:            return "OPEN";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2_WPA3_PSK";
        case WIFI_AUTH_WAPI_PSK:        return "WAPI_PSK";
        case WIFI_AUTH_OWE:             return "OWE";
        default:                        return "UNKNOWN";
    }
}

/**
 * @brief Event handler for WiFi scan done event
 */
static void wifi_scan_done_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    if (s_scanner_event_group != NULL) {
        xEventGroupSetBits(s_scanner_event_group, SCAN_DONE_BIT);
    }
}

/**
 * @brief Process and log scan results
 */
static void process_scan_results(void)
{
    uint16_t ap_count = 0;
    esp_err_t err;

    /* Get the number of APs found */
    err = esp_wifi_scan_get_ap_num(&ap_count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get AP count: %s", esp_err_to_name(err));
        return;
    }

    if (ap_count == 0) {
        ESP_LOGI(TAG, "No access points found");
        return;
    }

    /* Limit to max AP count */
    uint16_t fetch_count = (ap_count > WIFI_SCANNER_MAX_AP_COUNT) 
                           ? WIFI_SCANNER_MAX_AP_COUNT 
                           : ap_count;

    /* Allocate memory for AP records */
    wifi_ap_record_t *ap_records = malloc(fetch_count * sizeof(wifi_ap_record_t));
    if (ap_records == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for AP records");
        /* Clear the scan results even on error */
        esp_wifi_scan_get_ap_records(&fetch_count, NULL);
        return;
    }

    /* Retrieve the AP records */
    err = esp_wifi_scan_get_ap_records(&fetch_count, ap_records);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get AP records: %s", esp_err_to_name(err));
        free(ap_records);
        return;
    }

    /* Log the results */
    ESP_LOGI(TAG, "Found %u access points:", ap_count);
    ESP_LOGI(TAG, "%-32s | %4s | %2s | %-15s", "SSID", "RSSI", "CH", "Auth");
    ESP_LOGI(TAG, "--------------------------------------------------------------");

    for (int i = 0; i < fetch_count; i++) {
        /* Handle hidden SSIDs */
        const char *ssid = (strlen((char *)ap_records[i].ssid) > 0) 
                           ? (char *)ap_records[i].ssid 
                           : "<hidden>";
        
        ESP_LOGI(TAG, "%-32s | %4d | %2d | %-15s",
                 ssid,
                 ap_records[i].rssi,
                 ap_records[i].primary,
                 auth_mode_to_str(ap_records[i].authmode));
    }

    if (ap_count > fetch_count) {
        ESP_LOGI(TAG, "... and %u more (not shown)", ap_count - fetch_count);
    }

    free(ap_records);
}

/**
 * @brief WiFi scanner background task
 */
static void wifi_scanner_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Scanner task started, interval: %lu seconds", (unsigned long)s_scan_interval_sec);

    /* Configure scan parameters */
    wifi_scan_config_t scan_config = {
        .ssid = NULL,               /* Scan all SSIDs */
        .bssid = NULL,              /* Scan all BSSIDs */
        .channel = 0,               /* Scan all channels */
        .show_hidden = true,        /* Include hidden networks */
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 400,         /* Min active scan time per channel (ms) */
                .max = 900          /* Max active scan time per channel (ms) */
            },
            .passive = 360          /* Passive scan time per channel (ms) */
        }
    };

    while (s_scanner_running) {
        ESP_LOGI(TAG, "Starting WiFi scan...");

        /* Clear any previous event bits */
        xEventGroupClearBits(s_scanner_event_group, SCAN_DONE_BIT);

        /* Start non-blocking scan */
        esp_err_t err = esp_wifi_scan_start(&scan_config, false);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start scan: %s", esp_err_to_name(err));
            /* Wait before retrying */
            vTaskDelay(pdMS_TO_TICKS(s_scan_interval_sec * 1000));
            continue;
        }

        /* Wait for scan to complete (with timeout) */
        EventBits_t bits = xEventGroupWaitBits(
            s_scanner_event_group,
            SCAN_DONE_BIT,
            pdTRUE,                  /* Clear bits on exit */
            pdFALSE,                 /* Wait for any bit */
            pdMS_TO_TICKS(30000)    /* 30 second timeout */
        );

        if (bits & SCAN_DONE_BIT) {
            /* Scan completed, process results */
            process_scan_results();
        } else {
            ESP_LOGW(TAG, "Scan timed out");
            /* Stop any ongoing scan */
            esp_wifi_scan_stop();
        }

        /* Wait for the configured interval before next scan */
        ESP_LOGD(TAG, "Next scan in %lu seconds", (unsigned long)s_scan_interval_sec);
        vTaskDelay(pdMS_TO_TICKS(s_scan_interval_sec * 1000));
    }

    ESP_LOGI(TAG, "Scanner task stopping");
    vTaskDelete(NULL);
}

esp_err_t wifi_scanner_init(void)
{
    if (s_scanner_task_handle != NULL) {
        ESP_LOGW(TAG, "Scanner already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    /* Create event group for scan synchronization */
    s_scanner_event_group = xEventGroupCreate();
    if (s_scanner_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    /* Register event handler for scan done */
    esp_err_t err = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                                wifi_scan_done_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register scan done handler: %s", esp_err_to_name(err));
        vEventGroupDelete(s_scanner_event_group);
        s_scanner_event_group = NULL;
        return err;
    }

    /* Create the scanner task */
    s_scanner_running = true;
    BaseType_t ret = xTaskCreate(
        wifi_scanner_task,
        "wifi_scanner",
        SCANNER_TASK_STACK_SIZE,
        NULL,
        SCANNER_TASK_PRIORITY,
        &s_scanner_task_handle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create scanner task");
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, wifi_scan_done_handler);
        vEventGroupDelete(s_scanner_event_group);
        s_scanner_event_group = NULL;
        s_scanner_running = false;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "WiFi scanner initialized");
    return ESP_OK;
}

esp_err_t wifi_scanner_set_interval(uint32_t seconds)
{
    if (seconds < MIN_SCAN_INTERVAL_SEC) {
        ESP_LOGE(TAG, "Scan interval must be at least %d seconds", MIN_SCAN_INTERVAL_SEC);
        return ESP_ERR_INVALID_ARG;
    }

    s_scan_interval_sec = seconds;
    ESP_LOGI(TAG, "Scan interval set to %lu seconds", (unsigned long)seconds);
    return ESP_OK;
}

uint32_t wifi_scanner_get_interval(void)
{
    return s_scan_interval_sec;
}

esp_err_t wifi_scanner_stop(void)
{
    if (s_scanner_task_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    /* Signal task to stop */
    s_scanner_running = false;

    /* Give the task time to exit gracefully */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Unregister event handler */
    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, wifi_scan_done_handler);

    /* Clean up event group */
    if (s_scanner_event_group != NULL) {
        vEventGroupDelete(s_scanner_event_group);
        s_scanner_event_group = NULL;
    }

    s_scanner_task_handle = NULL;
    ESP_LOGI(TAG, "WiFi scanner stopped");
    return ESP_OK;
}
