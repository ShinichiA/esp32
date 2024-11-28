#ifndef _NVS_COMPONENTS_H_
#define _NVS_COMPONENTS_H_

#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define NVS_NAMESPACE "wifi_storage"  // Tên namespace (khu vực lưu trữ)
#define MAX_SSID_LEN 32               // Độ dài tối đa cho SSID
#define MAX_PASS_LEN 64               // Độ dài tối đa cho mật khẩu
#define WIFI_SSID_KEY "ssid"  			// Key lưu SSID
#define WIFI_PASSWORD_KEY "password"  	// Key lưu mật khẩu Wi-Fi

void nvs_init(void);
esp_err_t open_nvs();
void close_nvs();

esp_err_t get_wifi_login_info(char *ssid, char *password);
esp_err_t write_wifi_login_info(char *ssid, char *password);

bool check_nvs_is_open();

#endif