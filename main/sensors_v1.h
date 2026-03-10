#ifndef SENSORS_H
#define SENSORS_H

#include "esp_err.h"

#define I2C_MASTER_SCL_IO 9
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_NUM 0
#define I2C_MASTER_FREQ_HZ 100000

#define SPI_MISO_PIN 37 
#define SPI_MOSI_PIN 35
#define SPI_CLK_PIN 36
#define SPI_CS_PIN 38   

void sensors_init(void);
void sensors_task(void *pvParameters);

#endif
