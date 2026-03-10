#ifndef MOTOR_H
#define MOTOR_H

#include "esp_err.h"

#define SERVO_PULSE_GPIO 5

void motor_init(void);
void motor_set_angle(int angle);
void servo_test_task(void *pvParameters);
#endif
