#include "sensors_v1.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define AHT20_ADDR 0x38
#define LSM6DS3_WHO_AM_I_REG 0x0F

// змінні які я тупо забув..
float temperature = 0.0f;
float humidity = 0.0f;
uint8_t who_am_i = 0;
int16_t acc_x = 0, acc_y = 0, acc_z = 0;

static const char *TAG = "SENSORS";

spi_device_handle_t spi_lsm6ds3_handle;

esp_err_t lsm6ds3_write_reg(uint8_t reg, uint8_t data) {uint8_t tx_data[2] = {reg & 0x7F, data}; 
    spi_transaction_t trans = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(spi_lsm6ds3_handle, &trans);
}

esp_err_t lsm6ds3_read_accel(int16_t *acc_x, int16_t *acc_y, int16_t *acc_z) {
    uint8_t tx_data[7] = {0x28 | 0x80, 0, 0, 0, 0, 0, 0}; 
    uint8_t rx_data[7] = {0};

    spi_transaction_t trans = {
        .length = 7 * 8, // 1 байт команди + 6 байтів для X, Y, Z
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };

    esp_err_t ret = spi_device_transmit(spi_lsm6ds3_handle, &trans);
    *acc_x = (int16_t)((rx_data[2] << 8) | rx_data[1]);
    *acc_y = (int16_t)((rx_data[4] << 8) | rx_data[3]);
    *acc_z = (int16_t)((rx_data[6] << 8) | rx_data[5]);

    return ret;
}



esp_err_t aht20_read_temp_hum(float *temperature, float *humidity) {
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    i2c_master_write_to_device(I2C_MASTER_NUM, AHT20_ADDR, cmd, 3, pdMS_TO_TICKS(100));
    vTaskDelay(pdMS_TO_TICKS(80));
    
    uint8_t data[6];
    i2c_master_read_from_device(I2C_MASTER_NUM, AHT20_ADDR, data, 6, pdMS_TO_TICKS(100));

    uint32_t hum_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t temp_raw = (((uint32_t)(data[3] & 0x0F)) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity = (float)hum_raw * 100.0f / 1048576.0f;
    *temperature = ((float)temp_raw * 200.0f / 1048576.0f) - 50.0f;

    return ESP_OK;
}

esp_err_t lsm6ds3_read_who_am_i(uint8_t *who_am_i) {
    uint8_t tx_data[2] = {LSM6DS3_WHO_AM_I_REG | 0x80, 0x00};
    uint8_t rx_data[2] = {0};

    spi_transaction_t trans = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };

    esp_err_t ret = spi_device_transmit(spi_lsm6ds3_handle, &trans);
    *who_am_i = rx_data[1];
    
    return ret;
}


void sensors_init(void) {
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0));

    spi_bus_config_t spi_bus_conf = {
        .miso_io_num = SPI_MISO_PIN,
        .mosi_io_num = SPI_MOSI_PIN,
        .sclk_io_num = SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_conf, SPI_DMA_CH_AUTO));
    
    spi_device_interface_config_t dev_conf = {
        .clock_speed_hz = 1000000,
        .mode = 3,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_conf, &spi_lsm6ds3_handle));

    ESP_LOGI(TAG, "I2C and SPI initialized");
    }


    

void sensors_task(void *pvParameters) {
    lsm6ds3_write_reg(0x10, 0x40);
    while (1) {
     
        vTaskDelay(pdMS_TO_TICKS(1000));
        aht20_read_temp_hum(&temperature, &humidity);
       
        lsm6ds3_read_who_am_i(&who_am_i );
        lsm6ds3_read_accel(&acc_x, &acc_y, &acc_z);
        
        ESP_LOGI("SENSORS", "Temp: %.2f C, Hum: %.2f %%, Acc: X=%d Y=%d Z=%d", 
                 temperature, humidity, acc_x, acc_y, acc_z);    
    }
}
