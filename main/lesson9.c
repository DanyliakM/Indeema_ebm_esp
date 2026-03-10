#include <stdio.h>
#include <string.h>
#include "esp_console.h"
#include "esp_log.h"
#include "iot_servo.h"

#define SERVO_GPIO 5
void app_main(void) {
   servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin = {
                SERVO_GPIO,
            },
            .ch = {
                LEDC_CHANNEL_0,
            },
        },
        .channel_number = 1,
    };


    esp_err_t ret = iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE("SERVO", "Помилка ініціалізації серво!");
        return;
    }
    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 90.0f);
    ESP_LOGI("SERVO", "Серво встановлено на 90 градусів");
}   

