idf_component_register(SRCS "nvs_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash
                    REQUIRES esp_system
                    REQUIRES log
                    REQUIRES freertos
                    )

