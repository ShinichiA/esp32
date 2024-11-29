#include <string.h>
#include "arch/sys_arch.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi_components.h"

httpd_handle_t server = NULL;
wifi_mode_t mode;

static bool is_reconnect = true;
const char* wifi_config_html = 
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>ESP32 Configuration</title>\n"
"    <style>\n"
"        body {\n"
"            font-family: Arial, sans-serif;\n"
"            text-align: center;\n"
"            background-color: #f4f4f4;\n"
"            margin: 0;\n"
"            padding: 0;\n"
"        }\n"
"        .container {\n"
"            margin-top: 50px;\n"
"            padding: 20px;\n"
"            background: white;\n"
"            border-radius: 8px;\n"
"            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);\n"
"            display: inline-block;\n"
"        }\n"
"        h1 {\n"
"            color: #333;\n"
"        }\n"
"        label {\n"
"            display: block;\n"
"            margin: 15px 0 5px;\n"
"            font-weight: bold;\n"
"        }\n"
"        input {\n"
"            width: 80%;\n"
"            padding: 10px;\n"
"            margin-bottom: 15px;\n"
"            border: 1px solid #ccc;\n"
"            border-radius: 4px;\n"
"        }\n"
"        button {\n"
"            padding: 10px 20px;\n"
"            background-color: #28a745;\n"
"            color: white;\n"
"            border: none;\n"
"            border-radius: 4px;\n"
"            cursor: pointer;\n"
"        }\n"
"        button:hover {\n"
"            background-color: #218838;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class=\"container\">\n"
"        <h1>ESP32 Configuration</h1>\n"
"        <form action=\"/config\" method=\"post\">\n"
"            <label for=\"ssid\">WiFi SSID:</label>\n"
"            <input type=\"text\" id=\"ssid\" name=\"ssid\" required>\n"
"\n"
"            <label for=\"password\">Password:</label>\n"
"            <input type=\"password\" id=\"password\" name=\"password\" required>\n"
"\n"
"            <button type=\"submit\">Save</button>\n"
"        </form>\n"
"    </div>\n"
"</body>\n"
"</html>";

//static int s_retry_num = 0;
static const char *TAG = "wifi station";

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                is_reconnect = true;
                ESP_ERROR_CHECK(esp_wifi_connect());
                break;
            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "WiFi STA stop");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
            	if (is_reconnect){
					ESP_LOGI(TAG, "WiFi STA disconnected, reconnecting...");
					ESP_ERROR_CHECK(esp_wifi_connect());
				}
                break;
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WiFi AP started");
                break;
            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "WiFi AP stopped");
                break;
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
                ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac), event->aid);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
                ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d", MAC2STR(event->mac), event->aid);
                break;
            }
            default:
                ESP_LOGI(TAG, "Unhandled WiFi event: %d", (int)event_id);
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
                break;
            }
            default:
                ESP_LOGI(TAG, "Unhandled IP event: %d", (int)event_id);
                break;
        }
    }
}


void get_wifi_mac_address(char *mac_addr) {
  uint8_t mac[6];
  esp_err_t err = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
  if (err != ESP_OK) {
    printf("Failed to get Wi-Fi MAC Address: %s\n", esp_err_to_name(err));
    mac_addr = NULL;
  }

  sprintf(mac_addr, MACSTR, MAC2STR(mac));
  printf("Wi-Fi MAC Address: %s\n", mac_addr);
}

void wifi_init(void) {
    // Khởi tạo các tài nguyên chung chỉ một lần
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_LOGI(TAG, "WiFi initialization complete.");
}

void wifi_stop(){
	is_reconnect = false;
	ESP_ERROR_CHECK(esp_wifi_stop()); // Dừng WiFi trước khi chuyển chế độ
}

void wifi_init_sta(char *ssid, char *password) {
    // Tạo netif STA nếu chưa có
    static esp_netif_t *sta_netif = NULL;
    if (!sta_netif) {
        sta_netif = esp_netif_create_default_wifi_sta();
    }
	
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Switched to STA mode. SSID:%s", ssid);
}

void wifi_init_ap(void) {
    // Tạo netif AP nếu chưa có
    static esp_netif_t *ap_netif = NULL;
    if (!ap_netif) {
        ap_netif = esp_netif_create_default_wifi_ap();
    }

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_ESP_WIFI_SSID,
            .ssid_len = strlen(AP_ESP_WIFI_SSID),
            .channel = AP_ESP_WIFI_CHANNEL,
            .password = AP_ESP_WIFI_PASS,
            .max_connection = AP_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    if (strlen(AP_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Switched to AP mode. SSID:%s channel:%d", AP_ESP_WIFI_SSID, AP_ESP_WIFI_CHANNEL);
}

// Hàm xử lý yêu cầu HTTP và trả về trang cấu hình
esp_err_t wifi_config_page_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, wifi_config_html, strlen(wifi_config_html));
    return ESP_OK;
}

// Hàm xử lý yêu cầu POST để cập nhật thông tin Wi-Fi
esp_err_t wifi_config_post_handler(httpd_req_t *req) {
    char buf[200] = {0};
    int ret;
    char *ssid = NULL, *password = NULL;

    // Đọc dữ liệu từ body của yêu cầu POST
    ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to receive request data");
        return ESP_FAIL;
    }

    // Tách SSID và Password từ dữ liệu nhận được
    char *token = strtok(buf, "&");
    while (token != NULL) {
        if (strncmp(token, "ssid=", 5) == 0) {
            ssid = token + 5; // Lấy phần sau "ssid="
        } else if (strncmp(token, "password=", 9) == 0) {
            password = token + 9; // Lấy phần sau "password="
        }
        token = strtok(NULL, "&");
    }

    ESP_LOGI(TAG, "Received SSID: %s, Password: %s", ssid, password);
    esp_wifi_get_mode(&mode);
	if (mode == WIFI_MODE_STA){
		// Trả về kết quả
	    const char* resp_str = "<html><body><h1>Please Go To Config Mode</h1></body></html>";
	    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
	    return ESP_OK;
	}
    // Lưu cấu hình Wi-Fi vào bộ nhớ NVS (hoặc áp dụng cấu hình này vào Wi-Fi STA mode)
	write_wifi_login_info(ssid, password);
	
    // Trả về kết quả
    const char* resp_str = "<html><body><h1>Configuration Saved!</h1></body></html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    // Có thể chuyển sang chế độ STA và kết nối Wi-Fi ở đây
    // stop_webserver();
    vTaskDelay(pdMS_TO_TICKS(1000));

    wifi_stop();
    vTaskDelay(pdMS_TO_TICKS(10000));
    wifi_init_sta(ssid, password);

    return ESP_OK;
}

// Hàm khởi tạo web server
httpd_handle_t start_webserver(void) {
	if (server != NULL){
		return server;
	}

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_resp_headers = 4096; // Increase header length
	config.max_uri_handlers = 1024;
	config.stack_size = 8096;
    // Cấu hình URL cho các yêu cầu GET và POST
    httpd_uri_t uri_get = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = wifi_config_page_get_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t uri_post = {
        .uri       = "/config",
        .method    = HTTP_POST,
        .handler   = wifi_config_post_handler,
        .user_ctx  = NULL
    };

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_post);

    return server;
}


void stop_webserver(void) {
    if (server) {
        httpd_stop(server);  // Dừng webserver
        server = NULL;
        ESP_LOGI("WEB", "Webserver stopped.");
    }
}

int get_current_wifi_mode(){
	esp_wifi_get_mode(&mode);
	return mode;
;
}