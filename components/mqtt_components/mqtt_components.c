#include <stdio.h>
#include "esp_err.h"
#include "mqtt_components.h"

static const char *TAG = "mqtt";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld" PRIi32 "", base, (long)event_id);
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI("MQTT", "Connected to broker");
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI("MQTT", "Disconnected from broker");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI("MQTT", "Subscribed to topic");
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI("MQTT", "Unsubscribed from topic");
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI("MQTT", "Message published");
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI("MQTT", "Received data on topic %s", event->topic);
    ESP_LOGI("MQTT", "Data: %d - %s", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI("MQTT", "Error occurred");
    break;
  default:
    ESP_LOGI("MQTT", "Unhandled event id: %ld", (long)event_id);
    break;
  }
}

esp_mqtt_client_handle_t mqtt_connect(char *uri) {
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = uri
      //.broker.address.hostname = "192.168.0.102",
      //.broker.address.port = 1883,
      //.cert_pem = your_cert_pem_start,  // Cung cấp chứng chỉ CA nếu cần
      //.client_cert_pem = your_client_cert_pem_start,
      //.client_key_pem = your_client_key_pem_start,
  };

  esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
  return mqtt_client;
}

void mqtt_publish(esp_mqtt_client_handle_t mqtt_client, char *topic, char *data, int length_data, int qos, int retain){
	esp_mqtt_client_publish(mqtt_client, topic, data, length_data, qos, retain);
}

