#include "servo_motor.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "joystick_led_v1.h" // для доступу до глобальної змінної global_joy_x

static const char *TAG = "MOTOR";

#define SERVO_MIN_PULSEWIDTH_DUTY 200
#define SERVO_MAX_PULSEWIDTH_DUTY 1000
#define ADC_CENTER 2048
#define DEADZONE 250

extern int global_joy_x;
extern int16_t global_accel_x;

void motor_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 50,  
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_PULSE_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    ESP_LOGI(TAG, "Servo motor initialized on GPIO %d", SERVO_PULSE_GPIO);
}
void motor_set_angle(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    int duty = SERVO_MIN_PULSEWIDTH_DUTY + (angle * (SERVO_MAX_PULSEWIDTH_DUTY - SERVO_MIN_PULSEWIDTH_DUTY)) / 180;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    ESP_LOGI(TAG, "Set servo angle to %d degrees (duty: %d)", angle, duty);
}

void servo_test_task(void *pvParameters) {
    while(1) {
       int target_angle = 90;
       if (global_joy_x > (ADC_CENTER + DEADZONE) || global_joy_x < (ADC_CENTER - DEADZONE)) {
        target_angle = (global_joy_x * 180) / 4095;
        } else {
            int accel_corr = (global_accel_x * 45) / 16384; // Корекція в межах +-45 градусів
            target_angle = 90 + accel_corr;
        }
        motor_set_angle(target_angle);
        if (global_joy_x % 100 == 0) { 
            ESP_LOGD(TAG, "Joy: %d, Accel: %d -> Target: %d", global_joy_x, global_accel_x, target_angle);
        }
        vTaskDelay(pdMS_TO_TICKS(50));  
    }
}
