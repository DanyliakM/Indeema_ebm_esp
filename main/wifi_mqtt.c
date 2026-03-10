#include "wifi_mqtt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "joystick_led_v1.h"
#include <string.h>

#define WIFI_SSID "халява"
#define WIFI_PASS "12344567"
#define MQTT_BROKER_URI "mqtt://broker.hivemq.com"

static const char *TAG = "WIFI_MQTT";
static EventGroupHandle_t s_wifi_event_group;
esp_mqtt_client_handle_t client;

extern volatile bool mqtt_override; // для можливості вимикати MQTT при натисканні кнопки джойстика

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(s_wifi_event_group, BIT0, pdFALSE, pdFALSE, portMAX_DELAY);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    if (event_id == MQTT_EVENT_CONNECTED) {
        esp_mqtt_client_subscribe(client, "indeema/esp32/command", 0);
    } else if (event_id == MQTT_EVENT_DATA) {
        ESP_LOGI(TAG, "MQTT Command Received: %.*s", event->data_len, event->data);

        char *payload = malloc(event->data_len + 1);
        if (payload) {
            memcpy(payload, event->data, event->data_len);
            payload[event->data_len] = '\0';

            int r = 0, g = 0, b = 0;
            if (strstr(payload, "AUTO") != NULL) {
                mqtt_override = false;
                ESP_LOGW("STATUS_UART", "EXECUTION SUCCESS: Returned to Joystick control");
            } else if (sscanf(payload, "%d,%d,%d", &r, &g, &b) == 3) {
                mqtt_override = true;
                
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                if (r < 0) r = 0;
                if (g < 0) g = 0;
                if (b < 0) b = 0;

                set_led_color(r, g, b);
                ESP_LOGW("STATUS_UART", "EXECUTION SUCCESS: LED set to R:%d G:%d B:%d via MQTT", r, g, b);
            } else {
                ESP_LOGE("STATUS_UART", "ERROR: Unknown format. Send 'AUTO' or 'R,G,B' (e.g. '255,0,0')");
            }
            free(payload);
        }
    }
}


void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}