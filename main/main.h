#ifndef _MAIN_H_
#define _MAIN_H_


#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "gpio_components.h"
#include "mqtt_components.h"
#include "ota_components.h"
#include "wifi_components.h"

#define BLINK_GPIO GPIO_NUM_26
#define FIRMWARE_VERSION "1.0.0"
#define TURN_ON_LED 1
#define TURN_OFF_LED 0

#define TOPIC "device/voltage"

esp_mqtt_client_handle_t mqtt_client;

// receive buffer
char mac_addr[17];
char mqtt_uri[] = "mqtt://192.168.0.102:1883";

#endif
