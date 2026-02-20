#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "gatt_svr.h"
#include "esp_log.h"

static const char *TAG = "GATT_SVR";

// Наші кастомні ідентифікатори (UUID)
#define CUSTOM_SERVICE_UUID 0xABCD
#define CUSTOM_CHR_UUID     0x1111

// Ця функція спрацює, коли телефон надішле дані
static int custom_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        if (len > 0) {
            uint8_t *data = ctxt->om->om_data;
            ESP_LOGI(TAG, "Отримано %d байт. Значення: %c", len, data[0]);

            // Виконуємо дію на основі отриманого символу
            if (data[0] == '1') {
                ESP_LOGI(TAG, "👉 ДІЯ: УВІМКНУТИ (1)");
                // Тут можна додати виклик функції для esptool_led_strip
            } else if (data[0] == '0') {
                ESP_LOGI(TAG, "👉 ДІЯ: ВИМКНУТИ (0)");
                // Тут можна додати виклик вимкнення
            } else {
                ESP_LOGW(TAG, "Невідома команда: %c", data[0]);
            }
        }
    }
    return 0; // 0 означає успіх
}

// Створюємо таблицю сервісів та характеристик
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(CUSTOM_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(CUSTOM_CHR_UUID),
                .access_cb = custom_chr_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE, // Дозволяємо лише запис із телефону
            },
            {
                0, // Кінець списку характеристик
            }
        },
    },
    {
        0, // Кінець списку сервісів
    },
};

// Реєструємо нашу таблицю в NimBLE
int gatt_svr_init(void) {
    int rc;
    
    // Ініціалізуємо базові сервіси GAP і GATT
    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    // Додаємо наші власні сервіси
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) return rc;
    
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) return rc;
    
    return 0;
}