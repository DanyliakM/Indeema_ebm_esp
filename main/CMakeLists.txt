idf_component_register(SRCS "main_ble.c" "gatt_svr.c"
                       INCLUDE_DIRS "."
                            REQUIRES nvs_flash bt led_strip)