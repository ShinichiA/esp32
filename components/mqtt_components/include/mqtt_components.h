#ifndef _MQTT_COMPONENTS_H_
#define _MQTT_COMPONENTS_H_

#include "mqtt_client.h"
#include "esp_log.h"

esp_mqtt_client_handle_t mqtt_connect(char *uri);
void mqtt_publish(esp_mqtt_client_handle_t mqtt_client, char *topic, char *data, int length_data, int qos, int retain);

#endif
