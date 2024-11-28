#include "main.h"
#include "esp_err.h"
#include "esp_wifi_types.h"
#include "nvs_components.h"

static const char *TAG = "main";

void IRAM_ATTR button_isr_handler(void *arg) {
  int pin = (int)arg;
  int level = gpio_get_level(pin);

  if (level == 0) {                          // Nút nhấn được bấm
    press_start_time = esp_timer_get_time(); // Lưu thời gian bấm
  } else {
    int64_t press_duration = (esp_timer_get_time() - press_start_time) /
                             1000; // Tính thời gian giữ (ms)
    if (press_duration >= HOLD_TIME_MS) {
      if (xSemaphoreTake(xMutexConfig, portMAX_DELAY) == pdTRUE) {
        button_held = true; // Đánh dấu đã giữ đủ thời gian
        xSemaphoreGive(xMutexConfig);
      }
    }
  }
}

// Device control task
void device_control_task(void *pvParameter) {
  ESP_LOGI(TAG, "Device Control Task........");
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  int received_value;
  char received_string[100] = {0};
  while (1) {
    if (get_current_wifi_mode() != WIFI_MODE_STA) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }
    if (xQueueReceive(queue, &received_value, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
        printf("Received: %d\n", received_value);
        sprintf(received_string, "%d", received_value);
        mqtt_publish(mqtt_client, TOPIC, received_string,
                     strlen(received_string), 2, 0);
        memset(received_string, 0, sizeof(received_string));
        xSemaphoreGive(xMutex);
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Check update task
// downloads every 30sec the json file with the latest firmware
void OTA_task(void *pvParameter) {
  ESP_LOGI(TAG, "OTA Task........");
  while (1) {
    if (get_current_wifi_mode() != WIFI_MODE_STA) {
      vTaskDelay(30000 / portTICK_PERIOD_MS);
      continue;
    }
    update_ota(mac_addr, FIRMWARE_VERSION);
    // update_device_status(URL_UPDATE_DEVICE_STATUS, mac_addr);
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

void Config_task(void *pvParameter) {
  ESP_LOGI(TAG, "CONFIG Task........");
  int data = 42;
  while (1) {
    if (button_held && get_current_wifi_mode() == WIFI_MODE_STA) {
      wifi_stop();
      vTaskDelay(pdMS_TO_TICKS(10000));
      wifi_init_ap();
      start_webserver();
      if (xSemaphoreTake(xMutexConfig, portMAX_DELAY) == pdTRUE) {
        button_held = false;
        xSemaphoreGive(xMutexConfig);
      }
    }
    if (xQueueSend(queue, &data, portMAX_DELAY) == pdPASS) {
      printf("Data sent: %d\n", data);
    };
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void app_main() {
  printf("Firmware %s\n\n", FIRMWARE_VERSION);
  // KHỞI TẠO PHÂN VÙNG NVS
  nvs_init();
  ESP_ERROR_CHECK(open_nvs());
  // create queue
  queue = xQueueCreate(20, sizeof(int));
  // create mutex
  xMutex = xSemaphoreCreateMutex();
  xMutexConfig = xSemaphoreCreateMutex();
  // init wifi
  wifi_init();
  char ssid[MAX_SSID_LENGTH] = {0};
  char password[MAX_PASSWORD_LENGTH] = {0};

  if (get_wifi_login_info(ssid, password) == ESP_OK) {
    wifi_init_sta(ssid, password);
  } else {
    wifi_init_ap();
    start_webserver();
  }
  get_wifi_mac_address(mac_addr);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  init_gpio(button_isr_handler);

  // update status
  // update_device_status(URL_UPDATE_DEVICE_STATUS, mac_addr);

  // mqtt config
  mqtt_client = mqtt_connect(MQTT_URI);

  //  start the check update task
  //  1024 <-> 4096 * 4 = 16384 byte = 16kb
  //  8192 <-> 8192 * 4 = 32768 byte = 32kb
  xTaskCreate(&device_control_task, "device_control_task", 4096, NULL, 1, NULL);
  xTaskCreate(&OTA_task, "OTA_task", 4096, NULL, 2, &OTA_Task_Handle);
  xTaskCreate(&Config_task, "CONFIG_task", 4096, NULL, 3, NULL);

  size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  size_t min_free_heap = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);

  ESP_LOGI("HEAP", "Current free heap size: %zu bytes", free_heap);
  ESP_LOGI("HEAP", "Minimum free heap size: %zu bytes", min_free_heap);
}
