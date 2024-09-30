#ifndef _API_COMPONENTS_H_
#define _API_COMPONENTS_H_

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

#define MAX_CONTENT_LENGTH_SIZE 1000

cJSON *get_requests_without_chunk(char *url);
cJSON *post_requests(char *url, cJSON *json_data);
esp_err_t update_device_status(char *mac);
#endif