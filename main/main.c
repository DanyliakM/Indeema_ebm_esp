#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "joystick_led_v1.h"
#include "wifi_mqtt.h"
#include "uart_handler.h"

int global_joy_x = 2048;
int16_t global_accel_x = 0;

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    leds_joystick_init();
    
    wifi_init_sta();
    mqtt_app_start();
    
    uart_handler_init();

    xTaskCreatePinnedToCore(leds_joystick_task, "joy_task", 4096, NULL, 5, NULL, 1);
}