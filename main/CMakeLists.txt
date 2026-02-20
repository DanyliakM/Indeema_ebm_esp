# Вказуємо нові файли: main_ble.c та gatt_svr.c
idf_component_register(SRCS "main_ble.c" "gatt_svr.c"
                       INCLUDE_DIRS "."
                       REQUIRES nvs_flash bt)