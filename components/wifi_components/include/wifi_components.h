#ifndef _WIFI_COMPONENTS_H_
#define _WIFI_COMPONENTS_H_

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#include "esp_http_server.h"
#include <stdint.h>

#define EXAMPLE_ESP_WIFI_SSID      "loadl"
#define EXAMPLE_ESP_WIFI_PASS      ""
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

#define WIFI_SSID "Phong_301"
#define WIFI_PASS "1234abcd"
#define ESP_MAXIMUM_RETRY 3

void wifi_init(void);
void wifi_init_sta();
void get_wifi_mac_address(char *mac_addr);

void wifi_stop();
void wifi_init_ap(void);
httpd_handle_t start_webserver(void);
void stop_webserver(void);
#endif