#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOSConfig.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "gatt_svr.h"

static const char *TAG = "MAIN_BLE";
static uint8_t own_addr_type;

void ble_app_advertise(void);

// Обробник подій підключення/відключення
static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI(TAG, "Телефон підключився! Статус: %d", event->connect.status);
            if (event->connect.status != 0) {
                ble_app_advertise(); // Якщо помилка - рекламуємо знову
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Телефон відключився! Причина: %d", event->disconnect.reason);
            ble_app_advertise(); // Знову починаємо пошук
            break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "Рекламування завершено, перезапуск...");
            ble_app_advertise();
            break;
    }
    return 0;
}

// Налаштування "видимості" для телефону
void ble_app_advertise(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    
    // Вказуємо ім'я, яке побачить телефон
    const char *name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Помилка налаштування реклами: %d", rc);
        return;
    }

    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_gap_event, NULL);
    if (rc == 0) {
        ESP_LOGI(TAG, "Пристрій рекламується як '%s'...", name);
    }
}

// Запускається, коли стек Bluetooth готовий до роботи
static void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, &own_addr_type);
    ble_app_advertise(); // Починаємо рекламувати
}

// Окремий потік FreeRTOS для обробки Bluetooth
void ble_host_task(void *param) {
    ESP_LOGI(TAG, "Запуск NimBLE Host Task");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// ГОЛОВНА ТОЧКА ВХОДУ (Замість main())
void app_main(void) {
    ESP_LOGI(TAG, "Запуск системи на ESP32-S3...");

    // 1. Очищення та ініціалізація NVS (Необхідно для роботи Bluetooth)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Ініціалізація ядра NimBLE
    nimble_port_init();

    // 3. Встановлюємо ім'я пристрою
    ble_svc_gap_device_name_set("Indeema_ESP_S3");

    // 4. Викликаємо нашу функцію з gatt_svr.c
    gatt_svr_init();

    // 5. Кажемо стеку, що робити після синхронізації
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    // 6. Запускаємо Bluetooth у його власному потоці
    nimble_port_freertos_init(ble_host_task);
}