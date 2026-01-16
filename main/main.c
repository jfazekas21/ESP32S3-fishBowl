/**
 * @file main.c
 * @brief Application entry point - initializes WiFi and starts the scanner task
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_scanner.h"

static const char *TAG = "main";

/**
 * @brief Initialize NVS flash storage
 * 
 * NVS is required for WiFi calibration data storage.
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

/**
 * @brief Initialize WiFi in station mode for scanning
 */
static esp_err_t init_wifi(void)
{
    esp_err_t ret;

    /* Initialize the TCP/IP stack */
    ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize netif: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Create the default event loop */
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Create default WiFi station interface */
    esp_netif_create_default_wifi_sta();

    /* Initialize WiFi with default configuration */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Set WiFi mode to station (required for scanning) */
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Start WiFi */
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "WiFi initialized in station mode");
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 FishBowl starting...");

    /* Initialize NVS - required for WiFi */
    ESP_ERROR_CHECK(init_nvs());
    ESP_LOGI(TAG, "NVS initialized");

    /* Initialize WiFi in station mode */
    ESP_ERROR_CHECK(init_wifi());

    /* Start the WiFi scanner background task */
    ESP_ERROR_CHECK(wifi_scanner_init());

    ESP_LOGI(TAG, "Initialization complete");

    /* Main task can now do other work or simply idle.
     * The WiFi scanner runs as a separate background task. */
}
