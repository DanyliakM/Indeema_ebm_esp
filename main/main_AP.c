#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"

/* Налаштування нашої точки доступу */
#define EXAMPLE_ESP_WIFI_SSID      "ESP32_ACCESS_POINT"
#define EXAMPLE_ESP_WIFI_PASS      "12345678" // Пароль мінімум 8 символів для WPA2
#define EXAMPLE_MAX_STA_CONN       4          // Максимальна кількість підключених клієнтів

static const char *TAG = "wifi softAP";

/* Обробник подій (Event Handler)
 * Тут ми відстежуємо, коли хтось підключається або відключається від нашої ESP32
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    // Якщо клієнт підключився до нашої точки доступу
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } 
    // Якщо клієнт відключився
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    
    // ОСЬ ТУТ БУЛА ПОМИЛКА: додано аргументи після коми
    // Додайте аргументи: MAC2STR(event->mac) та event->aid
    ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
}
}

void wifi_init_softap(void)
{
    // 1. Ініціалізуємо мережевий стек
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // ВАЖЛИВО: Створюємо AP (Access Point), а не STA
    esp_netif_create_default_wifi_ap();

    // 2. Ініціалізуємо драйвер Wi-Fi з налаштуваннями за замовчуванням
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Реєструємо обробник подій
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // 4. Налаштовуємо параметри нашої точки доступу
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = 1,                 // Канал Wi-Fi (1-13)
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK, // Тип шифрування
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    // Якщо пароль пустий, робимо мережу відкритою
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // 5. Запускаємо режим AP
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, wifi_config.ap.channel);
}

void app_main(void)
{
    //Initialize NVS (потрібно для роботи Wi-Fi драйвера)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
}