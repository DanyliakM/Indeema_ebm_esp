#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "gatt_svr.h"
#include "esp_log.h"
#include <time.h>

static const char *TAG = "GATT_SVR";

#define CUSTOM_SERVICE_UUID 0xABCD
#define CUSTOM_CHR_UUID     0x1111
extern void control_onboard_led(bool turn_on);

static int custom_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        if (len > 0) {
            uint8_t *data = ctxt->om->om_data;
            ESP_LOGI(TAG, "Get %d byte. meaning: %c", len, data[0]);

            if (data[0] == '1') {
                ESP_LOGI(TAG, "Turn on (1)");
                control_onboard_led(true);
            } else if (data[0] == '0') {
                ESP_LOGI(TAG, "Turn off (0)");
                control_onboard_led(false);
                
            } else {
                ESP_LOGW(TAG, "Unknown command: %c", data[0]);
            }
        }
    }
    return 0; 
}

static uint8_t battery_level = 85;



static int battery_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // Телефон просить ПРОЧИТАТИ дані
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        ESP_LOGI(TAG, "Phone asked for battery level");
        
        int rc = os_mbuf_append(ctxt->om, &battery_level, sizeof(battery_level));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return 0;
}





static int currTimeService_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        ESP_LOGI(TAG, "Телефон запитав час (Формат Exact Time 256)");
        
        // 1. Отримуємо сирі секунди
        time_t curr_time = time(NULL);
        
        // 2. Розбиваємо секунди на структуру з годинами, днями тощо
        struct tm timeinfo;
        localtime_r(&curr_time, &timeinfo); 
        
        // 3. Створюємо масив на 10 байт для Bluetooth
        uint8_t time_data[10];
        
        // Рік (у C tm_year рахує роки з 1900, тому додаємо 1900)
        uint16_t year = timeinfo.tm_year + 1900;
        time_data[0] = year & 0xFF;         // Молодший байт (наприклад, 0xDC для 2012)
        time_data[1] = (year >> 8) & 0xFF;  // Старший байт (наприклад, 0x07 для 2012)
        
        // Місяць (у C tm_mon від 0 до 11, а Bluetooth хоче 1-12)
        time_data[2] = timeinfo.tm_mon + 1;
        
        // День місяця
        time_data[3] = timeinfo.tm_mday;
        
        // Години, хвилини, секунди
        time_data[4] = timeinfo.tm_hour;
        time_data[5] = timeinfo.tm_min;
        time_data[6] = timeinfo.tm_sec;
        
        // День тижня (У C Неділя = 0, а в Bluetooth Неділя = 7)
        time_data[7] = (timeinfo.tm_wday == 0) ? 7 : timeinfo.tm_wday;
        
        // Долі секунди (залишаємо по нулях)
        time_data[8] = 0;
        
        // Причина зміни часу (0 = невідомо / ручне налаштування)
        time_data[9] = 0;

        // 4. Відправляємо всі 10 байт
        int rc = os_mbuf_append(ctxt->om, time_data, sizeof(time_data));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return 0;
}


static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(CUSTOM_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(CUSTOM_CHR_UUID),
                .access_cb = custom_chr_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE, 
            },
            { 0 } 
        },
    },

   
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180F), // Стандартний UUID для Battery Service
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0x2A19), // Стандартний UUID для Battery Level
                .access_cb = battery_chr_access_cb, // Наш новий колбек
                .flags = BLE_GATT_CHR_F_READ,       // Дозволяємо телефону ЧИТАТИ рівень
            },
            { 0 } // Кінець характеристик сервісу батареї
        },
    },

    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x1805), // Стандартний UUID для Current Time Service
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0x2A2B), // Стандартний UUID для Current Time
                .access_cb = currTimeService_chr_access_cb, // Колбек для поточного часу
                .flags = BLE_GATT_CHR_F_READ,       // Дозволяємо телефону ЧИТАТИ поточний час
            },
            { 0 } // Кінець характеристик сервісу поточного часу
        },
    },
    { 0 }
};

int gatt_svr_init(void) {
    int rc;
    
    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) return rc;
    
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) return rc;
    
    return 0;
}