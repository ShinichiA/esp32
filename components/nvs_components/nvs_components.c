#include "nvs_components.h"
#include "esp_err.h"
#include <stdio.h>

static bool is_nvs_open = false;
nvs_handle_t _nvs_handle = 0; // Biến lưu trữ NVS handle
SemaphoreHandle_t xMutexNVS;

void nvs_init(void) {
  esp_err_t err = nvs_flash_init();
  ESP_ERROR_CHECK(err);
  xMutexNVS = xSemaphoreCreateMutex();
}

esp_err_t get_wifi_login_info(char *ssid, char *password) {
  esp_err_t err;
  size_t length_ssid = MAX_SSID_LEN;
  size_t length_password = MAX_SSID_LEN;
  if (xSemaphoreTake(xMutexNVS, portMAX_DELAY) == pdTRUE) {
    err = nvs_get_str(_nvs_handle, WIFI_SSID_KEY, ssid, &length_ssid);
    if (err == ESP_OK) {
      ESP_LOGI("NVS", "Read SSID from NVS: %s", ssid);
    } else {
      ESP_LOGE("NVS", "Failed to read SSID from NVS");
      xSemaphoreGive(xMutexNVS);
      return ESP_FAIL;
    }

    err =
        nvs_get_str(_nvs_handle, WIFI_PASSWORD_KEY, password, &length_password);
    if (err == ESP_OK) {
      ESP_LOGI("NVS", "Read password from NVS: %s", password);
    } else {
      ESP_LOGE("NVS", "Failed to read password from NVS");
      xSemaphoreGive(xMutexNVS);
      return ESP_FAIL;
    }
    xSemaphoreGive(xMutexNVS);
  }
  return ESP_OK;
}

esp_err_t write_wifi_login_info(char *ssid, char *password) {
  esp_err_t err;
  if (xSemaphoreTake(xMutexNVS, portMAX_DELAY) == pdTRUE) {
    // Lưu SSID vào NVS
    err = nvs_set_str(_nvs_handle, WIFI_SSID_KEY, ssid);
    if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to write SSID to NVS");
      xSemaphoreGive(xMutexNVS);
      return ESP_FAIL;
    }

    // Lưu mật khẩu vào NVS
    err = nvs_set_str(_nvs_handle, WIFI_PASSWORD_KEY, password);
    if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to write password to NVS");
      xSemaphoreGive(xMutexNVS);
      return ESP_FAIL;
    }

    // Commit dữ liệu vào NVS
    err = nvs_commit(_nvs_handle);
    if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to commit data to NVS");
      xSemaphoreGive(xMutexNVS);
      return ESP_FAIL;
    }
  }
  return ESP_OK;
}

esp_err_t open_nvs() {
  if (is_nvs_open) {
    ESP_LOGW("NVS", "NVS is already open");
    return ESP_OK;
  }

  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &_nvs_handle);
  if (err == ESP_OK) {
    is_nvs_open = true; // Đánh dấu là NVS đã mở thành công
    ESP_LOGI("NVS", "NVS opened successfully");
  } else {
    ESP_LOGE("NVS", "Failed to open NVS (%s)", esp_err_to_name(err));
  }
  return err;
}

void close_nvs() {
  if (!is_nvs_open) {
    ESP_LOGW("NVS", "NVS is not open");
    return;
  }

  nvs_close(_nvs_handle);
  is_nvs_open = false; // Đánh dấu là NVS đã đóng
  ESP_LOGI("NVS", "NVS closed successfully");
}

bool check_nvs_is_open(){
	return is_nvs_open;
}