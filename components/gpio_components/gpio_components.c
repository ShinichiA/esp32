#include <stdio.h>
#include "gpio_components.h"

void init_gpio(void (*isr_handler)(void*))
{
    // Cấu hình GPIO cho nút nhấn
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io_conf);

    // Cài đặt hàm xử lý ngắt cho nút nhấn
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_GPIO, isr_handler, (void*) BUTTON_GPIO);
}
