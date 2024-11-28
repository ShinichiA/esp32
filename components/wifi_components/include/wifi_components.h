#ifndef _WIFI_COMPONENTS_H_
#define _WIFI_COMPONENTS_H_

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#include "esp_http_server.h"
#include "nvs_components.h"
#include <stdint.h>

#define AP_ESP_WIFI_SSID      "ESP32_AP"
#define AP_ESP_WIFI_PASS      ""
#define AP_ESP_WIFI_CHANNEL   1
#define AP_MAX_STA_CONN       4

#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

void wifi_init(void);
void wifi_init_sta(char *ssid, char* password);
void get_wifi_mac_address(char *mac_addr);

void wifi_stop();
void wifi_init_ap(void);
httpd_handle_t start_webserver(void);
void stop_webserver(void);

int get_current_wifi_mode();

#endif