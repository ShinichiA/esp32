idf_component_register(SRCS "mqtt_components.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt
                    REQUIRES log)