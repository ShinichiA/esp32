idf_component_register(SRCS "ota_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES json
                    REQUIRES esp_wifi
                    REQUIRES api_components
                    REQUIRES esp_https_ota
                    REQUIRES nvs_flash
)
