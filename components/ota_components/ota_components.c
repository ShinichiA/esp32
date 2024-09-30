#include "ota_components.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "OTA";

esp_err_t _ota_callback(esp_http_client_handle_t client) {
  // có thể thêm header ở đây
  return ESP_OK;
}

void update_ota(char *mac_addr, char *firmware_version) {
  char *ota_url = malloc(MAX_LENGTH_URL + 1);
  
   if (ota_url == NULL) {
        // Handle memory allocation failure
        ESP_LOGE(TAG, "Failed to allocate memory");
        return;
    }
  printf("Looking for a new firmware...\n");
  // configure the esp_http_client
  memset(ota_url, 0, MAX_LENGTH_URL + 1);
  sprintf(ota_url, "%s?id=%s&version=%s", UPDATE_JSON_URL, mac_addr, firmware_version);
  cJSON *json = get_requests_without_chunk(ota_url);

  if (json == NULL)
    ESP_LOGI(TAG, "downloaded file is not a valid json, aborting...\n");
  else {
    cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
    cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");

    // check the version
    if (strcmp(version->valuestring, firmware_version) != 0) {
      ESP_LOGI(
          TAG,
          "current firmware version (%s) is difference (%s), upgrading...\n",
          firmware_version, version->valuestring);
      if (cJSON_IsString(file) && (file->valuestring != NULL)) {
        ESP_LOGI(TAG, "downloading and installing new firmware (%s)...\n",
                 file->valuestring);

        esp_http_client_config_t ota_client_config = {
            .url = file->valuestring,
            //.cert_pem = server_cert_pem_start,
        };
        esp_https_ota_config_t http_ota_config = {
            .http_config = &ota_client_config,
            .partial_http_download = true,
            .max_http_request_size = 0x1000,
            .bulk_flash_erase = true,
            .http_client_init_cb = _ota_callback,
        };
        esp_err_t ret = esp_https_ota(&http_ota_config);
        if (ret == ESP_OK) {
          ESP_LOGI(TAG, "OTA OK, restarting...\n");
          esp_restart();
        } else {
          ESP_LOGW(TAG, "OTA failed...\n");
        }
      } else
        ESP_LOGW(TAG, "unable to read the new file name, aborting...\n");
    } else
      ESP_LOGW(
          TAG,
          "current firmware version (%s) is equal (%s), nothing to do...\n",
          firmware_version, version->valuestring);
  }
  free(ota_url);
}
