// НАГАДУВАННЯ для себе - потім додай лед стрічку в v2 якщо буде час

#include "joystick_led_v1.h"
#include "led_strip.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "JOYSTICK";


extern int global_joy_x; // для коректної роботи сервомотора, щоб він реагував на джойстик, а не на акселерометр


static led_strip_handle_t led_strip;
static adc_oneshot_unit_handle_t adc1_handle; 

void leds_joystick_init(void) {
    led_strip_config_t strip_config = {
    .strip_gpio_num = LED_STRIP_GPIO,
    .max_leds = LED_STRIP_MAX_LEDS,
    .flags = {
        .invert_out = false,
    }
};
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_X_ADC_CHANNEL, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_Y_ADC_CHANNEL, &config));

    // потім змінити на іншу кнопку якщо буде час, поки що так для тесту
    gpio_config_t sw_config = {
        .pin_bit_mask = (1ULL << JOY_SW_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&sw_config));
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}


volatile bool mqtt_override = false; // для mqtt бо окремо робити запарно XD


void leds_joystick_task(void *pvParameters) {
    int x_val, y_val;
    uint8_t r = 0, g = 0, b = 0;
    uint8_t brightness = 50;

    while (1) {
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_X_ADC_CHANNEL, &x_val));
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_Y_ADC_CHANNEL, &y_val));

           // ESP_LOGI(TAG, "X: %d, Y: %d", x_val, y_val);
            
            global_joy_x = x_val; // для сервомотора
            if( !mqtt_override) {
            if (y_val < 1000) brightness = 0;
            else if (y_val > 3000) brightness = 100;
            else brightness = 20;

            if (x_val < 1000) { r = brightness; g = 0; b = 0; }
            else if (x_val > 3000) { r = 0; g = 0; b = brightness; }
            else { r = 0; g = brightness; b = 0; }

            set_led_color(r, g, b);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}