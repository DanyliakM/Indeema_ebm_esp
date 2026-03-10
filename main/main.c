/*
Послідовність при старті: 
. $HOME/esp/esp-idf/export.sh - запустити цей скрипт для налаштування середовища розробки
. idf.py set-target esp32s3 - встановити цільову платформу
. idf.py build - зібрати проект
. idf.py flash - прошити прошивку на пристрій
. idf.py monitor - відкрити монітор для перегляду виводу

Підключення ESP бо вилітає через WSL:
usbipd attach --wsl --busid 1-3 у PWSH 
sudo chmod 666 /dev/ttyACM0 - у VS Code cnsl
idf.py -p /dev/ttyACM0 flash monitor
*/



#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo_motor.h"
#include "esp_log.h"
#include "joystick_led_v1.h" // для доступу до глобальних змінних global_joy_x

int global_joy_x = 2048;
int16_t global_accel_x = 0;

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    motor_init();

    leds_joystick_init();

    xTaskCreatePinnedToCore(servo_test_task,  "servo_test_task", 2048, NULL, 5, NULL, 1);

    xTaskCreatePinnedToCore(leds_joystick_task, "joy_task", 4096, NULL, 5, NULL, 1);
}

