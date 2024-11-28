#ifndef _GPIO_COMPONENTS_H_
#define _GPIO_COMPONENTS_H_

#include "driver/gpio.h"
#include "esp_system.h"

#define ESP_INTR_FLAG_DEFAULT 10 // set level cho ngắt

#define BUTTON_GPIO GPIO_NUM_15 // GPIO cho nút nhấn
#define HOLD_TIME_MS 2000      // Thời gian giữ nút (ms)

void init_gpio(void (*isr_handler)(void*));
#endif