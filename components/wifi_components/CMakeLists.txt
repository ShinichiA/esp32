idf_component_register(SRCS "wifi_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi
                    REQUIRES nvs_flash
                    REQUIRES esp_http_server
                    REQUIRES log
                    REQUIRES esp_hw_support
                    REQUIRES nvs_components
)
