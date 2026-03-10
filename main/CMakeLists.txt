idf_component_register(SRCS "main.c"  "joystick_led_v1.c" "wifi_mqtt.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt nvs_flash esp_wifi esp_adc esp_event esp_netif)