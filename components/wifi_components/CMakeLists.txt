idf_component_register(SRCS "wifi_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi
                    REQUIRES nvs_flash
                    )
