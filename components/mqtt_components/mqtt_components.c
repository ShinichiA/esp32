#include <stdio.h>
#include "esp_err.h"
#include "mqtt_components.h"

static const char *TAG = "mqtt";

esp_mqtt_client_handle_t mqtt_connect(char *mac, const char *uri, void (*mqtt_event_handler_t)(void *, esp_event_base_t, int32_t, void *)) {
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = uri,
      .credentials.client_id = mac,
      .session.keepalive = 20,
      .session.disable_clean_session = true
      //.cert_pem = your_cert_pem_start,  // Cung cấp chứng chỉ CA nếu cần
      //.client_cert_pem = your_client_cert_pem_start,
      //.client_key_pem = your_client_key_pem_start,
  };

  esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler_t, NULL);
  esp_mqtt_client_start(mqtt_client);
  ESP_LOGI(TAG, "mqtt client start");
  return mqtt_client;
}

void mqtt_publish(esp_mqtt_client_handle_t mqtt_client, char *topic, char *data, int length_data, int qos, int retain){
	esp_mqtt_client_publish(mqtt_client, topic, data, length_data, qos, retain);
}

void mqtt_subcribe(esp_mqtt_client_handle_t mqtt_client, char *topic, int qos){
	esp_mqtt_client_subscribe(mqtt_client, topic, qos);
}
