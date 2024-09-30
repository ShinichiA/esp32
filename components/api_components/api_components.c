#include "api_components.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "api";

char URL_UPDATE_DEVICE_STATUS[] = "http://192.168.0.102:9000/api/device/status";

esp_err_t _https_event_handler(esp_http_client_event_t *evt) {
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
             evt->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, length=%d", evt->data_len);
    if (!esp_http_client_is_chunked_response(evt->client)) {
      if (evt->data_len > MAX_CONTENT_LENGTH_SIZE) {
        ESP_LOGI(
            TAG,
            "Content Length (%d) is bigger than MAX_CONTENT_LENGTH_SIZE (%d)",
            evt->data_len, MAX_CONTENT_LENGTH_SIZE);
        break;
      }
      strncpy(evt->user_data, (char *)evt->data, evt->data_len);
    }
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
    break;
  case HTTP_EVENT_REDIRECT:
    break;
  }
  return ESP_OK;
}

cJSON *get_requests_without_chunk(char *url) {
  char *buffer = malloc(MAX_CONTENT_LENGTH_SIZE + 1);
  memset(buffer, 0, MAX_CONTENT_LENGTH_SIZE + 1);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to create buffer");
    return NULL;
  }

  esp_http_client_config_t config = {.url = url,
                                     .method = HTTP_METHOD_GET,
                                     .event_handler = _https_event_handler,
                                     .user_data = buffer};
  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    free(buffer);
    return NULL;
  }

  // Lấy kích thước dữ liệu phản hồi
  int data_len = esp_http_client_get_content_length(client);
  ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
           esp_http_client_get_status_code(client), data_len);

  if (!esp_http_client_is_complete_data_received(client)) {
    ESP_LOGE(TAG, "Failed to receive buffer");
    free(buffer);
    return NULL;
  }

  buffer[data_len] = 0;
  ESP_LOGI(TAG, "esp_http_client_is_complete_data_received");
  ESP_LOGI(TAG, "Received data: %s", buffer);

  cJSON *json = cJSON_Parse(buffer);
  esp_http_client_cleanup(client);
  free(buffer);
  return json;
}

cJSON *post_requests(char *url, cJSON *json_data) {
  char *buffer = malloc(MAX_CONTENT_LENGTH_SIZE + 1);
  memset(buffer, 0, MAX_CONTENT_LENGTH_SIZE + 1);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to create buffer");
    return NULL;
  }

  // Convert cJSON object to string
  char *post_data = cJSON_PrintUnformatted(json_data);
  if (post_data == NULL) {
    ESP_LOGE(TAG, "Failed to create JSON string");
    return NULL;
  }

  esp_http_client_config_t config = {.url = url,
                                     .method = HTTP_METHOD_POST,
                                     .event_handler = _https_event_handler,
                                     .user_data = buffer};
  esp_http_client_handle_t client = esp_http_client_init(&config);
  // Set headers and POST data
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, post_data, strlen(post_data));

  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    free(buffer);
    return NULL;
  }

  // Lấy kích thước dữ liệu phản hồi
  int data_len = esp_http_client_get_content_length(client);
  ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
           esp_http_client_get_status_code(client), data_len);

  if (!esp_http_client_is_complete_data_received(client)) {
    ESP_LOGE(TAG, "Failed to receive buffer");
    free(buffer);
    free(post_data);
    return NULL;
  }

  buffer[data_len] = 0;
  ESP_LOGI(TAG, "esp_http_client_is_complete_data_received");
  ESP_LOGI(TAG, "Received data: %s", buffer);

  cJSON *json = cJSON_Parse(buffer);
  esp_http_client_cleanup(client);
  free(buffer);
  free(post_data);
  return json;
}
//----------------------------------------------------------------------------------------------------------

esp_err_t update_device_status(char *mac) {
  cJSON *json_data = cJSON_CreateObject();
  cJSON_AddStringToObject(json_data, "mac", mac);
  cJSON_AddBoolToObject(json_data, "status", true);
  cJSON *json_response = post_requests(URL_UPDATE_DEVICE_STATUS, json_data);
  if (!json_response) {
    free(json_response);
    free(json_data);
    return ESP_FAIL;
  }
  cJSON *status = cJSON_GetObjectItemCaseSensitive(json_response, "status");
  if (cJSON_IsTrue(status)) {
    free(json_response);
    free(json_data);
    return ESP_OK;
  }
  free(json_response);
  free(json_data);
  return ESP_FAIL;
}