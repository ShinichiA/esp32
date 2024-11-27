#include "main.h"
#include "api_components.h"
#include "arch/sys_arch.h"
#include "gpio_components.h"
#include "mqtt_components.h"
#include "wifi_components.h"
#include <string.h>

static const char *TAG = "main";
SemaphoreHandle_t xMutex;
static QueueHandle_t queue;

int get_data() { return 1; }

static void IRAM_ATTR button_isr_handler(void *arg) {
  int data = get_data();
  if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
    xQueueSend(queue, &data, pdMS_TO_TICKS(100));
    xSemaphoreGive(xMutex);
  }
}

// Device control task
void device_control_task(void *pvParameter) {
  ESP_LOGI(TAG, "Device Control Task........");
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  int received_value;
  char received_string[10];
  while (1) {
    if (xQueueReceive(queue, &received_value, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
        printf("Received: %d\n", received_value);
        sprintf(received_string, "%d", received_value);
        mqtt_publish(mqtt_client, TOPIC, received_string,
                     strlen(received_string), 2, 0);
        memset(received_string, 0, sizeof(received_string));
      }
    }
  }
}

// Check update task
// downloads every 30sec the json file with the latest firmware
void OTA_task(void *pvParameter) {
  ESP_LOGI(TAG, "OTA Task........");
  while (1) {
    update_ota(mac_addr, FIRMWARE_VERSION);
    update_device_status(mac_addr);
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

void app_main() {
  printf("Firmware %s\n\n", FIRMWARE_VERSION);
  // KHỞI TẠO PHÂN VÙNG NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      //ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
  ESP_ERROR_CHECK(ret);
  queue = xQueueCreate(20, sizeof(int));

  wifi_init();
  // connect to the wifi network
  wifi_init_sta();
  printf("Connected to wifi network\n");
  get_wifi_mac_address(mac_addr);
  vTaskDelay(pdMS_TO_TICKS(10000)); // 10 giây trong chế độ STA
  wifi_stop();
  vTaskDelay(pdMS_TO_TICKS(10000)); // 10 giây trong chế độ STA
  wifi_init_ap();
  start_webserver();
  vTaskDelay(pdMS_TO_TICKS(10000)); // 10 giây trong chế độ STA
  wifi_stop();
  vTaskDelay(pdMS_TO_TICKS(10000)); // 10 giây trong chế độ STA
  wifi_init_sta();

  // create mutex
  xMutex = xSemaphoreCreateMutex();
  init_gpio(button_isr_handler);
  // update status
  update_device_status(mac_addr);
  // mqtt config
  mqtt_client = mqtt_connect(mqtt_uri);
  //  start the check update task
  //  1024 <-> 4096 * 4 = 16384 byte = 16kb
  //  8192 <-> 8192 * 4 = 32768 byte = 32kb
  xTaskCreate(&device_control_task, "device_control_task", 4096, NULL, 5, NULL);
  xTaskCreate(&OTA_task, "OTA_task", 8192, NULL, 6, NULL);
}
