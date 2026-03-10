#ifndef LEDS_JOYSTICK_H
#define LEDS_JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "hal/adc_types.h"

#define LED_STRIP_GPIO 48 // може тут поміняю в v2, поки 48
#define LED_STRIP_MAX_LEDS 1 // НАГАДУВАННЯ ДЛЯ СЕБЕ - поміняй потім якщо додасиш лед-стрічку !
#define JOY_X_ADC_CHANNEL ADC_CHANNEL_0
#define JOY_Y_ADC_CHANNEL ADC_CHANNEL_1
#define JOY_SW_GPIO 4

void leds_joystick_init(void);
void leds_joystick_task(void *pvParameters);
void set_led_color(uint8_t r, uint8_t g, uint8_t b);

#endif