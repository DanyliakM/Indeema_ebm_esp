#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "gatt_svr.h"
#include "esp_log.h"

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
            {
                0, 
            }
        },
    },
    {
        0, 
    },
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