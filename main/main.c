/*
 * ESP32S3 FishBowl - Main Application
 * 
 * This is a base project for ESP32S3 using ESP-IDF v5.3
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "fishbowl";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32S3 FishBowl starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    while (1) {
        ESP_LOGI(TAG, "FishBowl running...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
