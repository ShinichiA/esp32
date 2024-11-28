#ifndef _OTA_COMPONENTS_H_
#define _OTA_COMPONENTS_H_

#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "api_components.h"
#include "nvs_flash.h"

#define MAX_LENGTH_URL 200
#define UPDATE_JSON_URL "http://192.168.0.101:9000/api/firmware"
void update_ota(char *mac_addr, char *firmware_version);

#endif
