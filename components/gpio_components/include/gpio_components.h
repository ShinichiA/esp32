#ifndef _GPIO_COMPONENTS_H_
#define _GPIO_COMPONENTS_H_

#include "driver/gpio.h"
#include "esp_system.h"

#define BUTTON_GPIO 20
#define ESP_INTR_FLAG_DEFAULT 10 // set level cho ngắt

void init_gpio(void (*isr_handler)(void*));
#endif