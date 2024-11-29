#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "arch/sys_arch.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "api_components.h"
#include "gpio_components.h"
#include "mqtt_components.h"
#include "nvs_components.h"
#include "ota_components.h"
#include "wifi_components.h"

#define URL_UPDATE_DEVICE_STATUS "http://192.168.0.101:9000/api/device/status"
#define MQTT_URI "mqtt://192.168.0.101:1883"
#define MQTT_MAX_LENGTH_DATA 200

#define BLINK_GPIO GPIO_NUM_26
#define FIRMWARE_VERSION "1.0.0"
#define TURN_ON_LED 1
#define TURN_OFF_LED 0

volatile int64_t press_start_time = 0; // Lưu thời gian bắt đầu bấm nút
volatile bool button_held = false;     // Cờ kiểm tra nút đã giữ 2s

#define TOPIC "device/voltage"

esp_mqtt_client_handle_t mqtt_client;
SemaphoreHandle_t xMutex;
SemaphoreHandle_t xMutexConfig;
SemaphoreHandle_t xMutexQueue;

QueueHandle_t queue;

TaskHandle_t OTA_Task_Handle;

// receive buffer
char mac_addr[17];
bool config_wifi = false;
char ssid[MAX_SSID_LENGTH] = {0};
char password[MAX_PASSWORD_LENGTH] = {0};
//static bool is_subcribe_topic = false;

#endif
