idf_component_register(SRCS "api_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client
                    REQUIRES json
)
