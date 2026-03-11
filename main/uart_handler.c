#include "uart_handler.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
#include "joystick_led_v1.h"

#define UART_PORT_NUM UART_NUM_0
#define BUF_SIZE 256

static void uart_task(void *arg) {
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    
    while (1) {
        int len = uart_read_bytes(UART_PORT_NUM, data, BUF_SIZE - 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';
            int r, g, b;
            
            if (sscanf((const char *)data, "%d,%d,%d", &r, &g, &b) == 3) {
                ESP_LOGI("UART_CMD", "R:%d G:%d B:%d", r, g, b);
                set_led_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
            }
        }
    }
}

void uart_handler_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    if (uart_is_driver_installed(UART_PORT_NUM)) {
        uart_driver_delete(UART_PORT_NUM);
    }

    uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
}