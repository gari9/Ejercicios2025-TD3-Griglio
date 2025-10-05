#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

static const char *TAG = "BT_SPP";
static uint32_t g_handle = 0;

// Callback SPP
static void spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch(event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(TAG, "SPP iniciado");
            esp_bt_gap_set_device_name("ESP32_SPP_SERVER");
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
            break;

        case ESP_SPP_START_EVT:
            ESP_LOGI(TAG, "Servidor SPP listo para conexiones");
            break;

        case ESP_SPP_SRV_OPEN_EVT:
            g_handle = param->srv_open.handle;
            ESP_LOGI(TAG, "Cliente conectado (handle=%u)", (unsigned)g_handle);
            break;

        case ESP_SPP_CLOSE_EVT:
            g_handle = 0;
            ESP_LOGI(TAG, "Cliente desconectado");
            break;

        case ESP_SPP_DATA_IND_EVT:
            ESP_LOGI(TAG, "Recibí (%u bytes): %.*s", param->data_ind.len,
                     param->data_ind.len, (char*)param->data_ind.data);
            if(g_handle) {
                const char ok[] = "OK, recibí: ";
                esp_spp_write(g_handle, sizeof(ok)-1, (uint8_t*)ok);
                esp_spp_write(g_handle, param->data_ind.len, param->data_ind.data);
            }
            break;

        default:
            break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Bluetooth clásico SPP...");

    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Liberar BLE (solo uso Classic BT)
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    // Inicializar controlador BT
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    // Inicializar Bluedroid
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // Registrar callback SPP y arrancar servidor
    ESP_ERROR_CHECK(esp_spp_register_callback(spp_cb));
    static const esp_spp_cfg_t spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size = 0
    };
    ESP_ERROR_CHECK(esp_spp_enhanced_init(&spp_cfg));

    ESP_LOGI(TAG, "Bluetooth listo. Buscame como 'ESP32_SPP_SERVER'.");
}
