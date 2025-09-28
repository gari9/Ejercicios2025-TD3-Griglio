/**
 * @file main.c
 * @brief WiFi con ESP32 y control de LED
 * @author Griglio Tomás    
 * @date 20/09/2025
 * 
 * 
 * Descripción:
 * Este programa implementa un sistema de aprovisionamiento WiFi para ESP32.
 * El dispositivo inicia como punto de acceso, permite la selección de una red
 * WiFi mediante interfaz web, y posteriormente actúa como cliente para 
 * proporcionar control remoto de un LED conectado al GPIO2.
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

/* Definiciones del sistema */
#define AP_SSID         "ESP32_Config"
#define AP_PASSWORD     "12345678"
#define AP_CHANNEL      1
#define MAX_SCAN_LIST   20
#define LED_GPIO        GPIO_NUM_2
#define MAX_RETRY       5

/* Prototipos de funciones */
static void inicializar_ap(void);
static void construir_pagina_escaneo(void);
static void iniciar_servidor_ap(void);
static void conectar_sta(const char *ssid, const char *password);
static void iniciar_servidor_sta(void);
static void manejador_eventos_wifi(void* arg, esp_event_base_t base, int32_t id, void* data);

/* Variables globales */
static const char *TAG = "WIFI_PROV";
static char buffer_html[4096];
static char ssid_objetivo[33];
static char password_objetivo[65];
static int estado_led = 0;
static bool solicitud_conexion_sta = false;

static httpd_handle_t servidor_ap = NULL;
static httpd_handle_t servidor_sta = NULL;

/* ==================== Manejadores HTTP - Modo AP ==================== */

static esp_err_t manejador_raiz_ap(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, buffer_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t manejador_conexion(httpd_req_t *req) {
    char buffer[2048] = {0};
    int longitud_total = req->content_len;
    int longitud_actual = 0;
    int recibido = 0;

    while (longitud_actual < longitud_total) {
        recibido = httpd_req_recv(req, buffer + longitud_actual, longitud_total - longitud_actual);
        if (recibido <= 0) {
            ESP_LOGE(TAG, "Error en recepción de datos POST");
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        longitud_actual += recibido;
    }
    buffer[longitud_actual] = '\0';

    ESP_LOGI(TAG, "Datos recibidos del formulario: %s", buffer);

    /* Parseo de datos del formulario */
    char *inicio_ssid = strstr(buffer, "ssid=");
    char *inicio_pass = strstr(buffer, "&pass=");
    
    if (inicio_ssid && inicio_pass) {
        inicio_ssid += 5;
        *inicio_pass = '\0';
        inicio_pass += 6;
        
        /* Decodificación URL básica */
        for (int i = 0; inicio_ssid[i]; i++) {
            if (inicio_ssid[i] == '+') inicio_ssid[i] = ' ';
        }
        
        strncpy(ssid_objetivo, inicio_ssid, sizeof(ssid_objetivo) - 1);
        strncpy(password_objetivo, inicio_pass, sizeof(password_objetivo) - 1);
        ssid_objetivo[sizeof(ssid_objetivo) - 1] = '\0';
        password_objetivo[sizeof(password_objetivo) - 1] = '\0';
        
    } else {
        ESP_LOGE(TAG, "Error en el análisis de credenciales");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Credenciales procesadas - SSID: %s", ssid_objetivo);

    const char *respuesta = "Configuración recibida. Iniciando conexión...";
    httpd_resp_sendstr(req, respuesta);

    solicitud_conexion_sta = true;
    return ESP_OK;
}

/* ==================== Manejadores HTTP - Modo STA ==================== */

static void generar_pagina_led(char *buffer, size_t tamaño) {
    snprintf(buffer, tamaño,
        "<!DOCTYPE html>"
        "<html><head><title>Control LED - ESP32</title>"
        "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'></head>"
        "<body style='font-family: Arial; text-align: center; margin: 50px;'>"
        "<h2>Control de LED - GPIO2</h2>"
        "<p>Estado actual: <strong>%s</strong></p>"
        "<div style='margin: 20px;'>"
        "<a href='/encender' style='display: inline-block; padding: 15px 30px; margin: 10px; background: #4CAF50; color: white; text-decoration: none; border-radius: 5px;'>ENCENDER</a>"
        "<a href='/apagar' style='display: inline-block; padding: 15px 30px; margin: 10px; background: #f44336; color: white; text-decoration: none; border-radius: 5px;'>APAGAR</a>"
        "</div>"
        "<hr><p><small>Sistema de control remoto ESP32</small></p>"
        "</body></html>",
        estado_led ? "ENCENDIDO" : "APAGADO");
}

static esp_err_t manejador_raiz_sta(httpd_req_t *req) {
    char pagina[1024];
    generar_pagina_led(pagina, sizeof(pagina));
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, pagina, strlen(pagina));
    return ESP_OK;
}

static esp_err_t manejador_encender(httpd_req_t *req) {
    estado_led = 1;
    gpio_set_level(LED_GPIO, 1);
    ESP_LOGI(TAG, "LED encendido mediante interfaz web");
    return manejador_raiz_sta(req);
}

static esp_err_t manejador_apagar(httpd_req_t *req) {
    estado_led = 0;
    gpio_set_level(LED_GPIO, 0);
    ESP_LOGI(TAG, "LED apagado mediante interfaz web");
    return manejador_raiz_sta(req);
}

/* ==================== Configuración de servidores HTTP ==================== */

static void iniciar_servidor_ap(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 2;
    config.max_uri_handlers = 8;
    config.recv_wait_timeout = 15;
    config.send_wait_timeout = 15;
    config.stack_size = 8192;
    config.max_resp_headers = 6;
    config.lru_purge_enable = true;
    
    if (httpd_start(&servidor_ap, &config) == ESP_OK) {
        httpd_uri_t uri_raiz = {.uri = "/", .method = HTTP_GET, .handler = manejador_raiz_ap};
        httpd_uri_t uri_conexion = {.uri = "/conectar", .method = HTTP_POST, .handler = manejador_conexion};
        
        httpd_register_uri_handler(servidor_ap, &uri_raiz);
        httpd_register_uri_handler(servidor_ap, &uri_conexion);
        
        ESP_LOGI(TAG, "Servidor HTTP iniciado en modo AP (192.168.4.1)");
    } else {
        ESP_LOGE(TAG, "Error al iniciar servidor HTTP en modo AP");
    }
}

static void iniciar_servidor_sta(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 6144;
    config.max_open_sockets = 3;
    config.lru_purge_enable = true;
    
    if (httpd_start(&servidor_sta, &config) == ESP_OK) {
        httpd_uri_t uri_raiz = {.uri = "/", .method = HTTP_GET, .handler = manejador_raiz_sta};
        httpd_uri_t uri_on = {.uri = "/encender", .method = HTTP_GET, .handler = manejador_encender};
        httpd_uri_t uri_off = {.uri = "/apagar", .method = HTTP_GET, .handler = manejador_apagar};
        
        httpd_register_uri_handler(servidor_sta, &uri_raiz);
        httpd_register_uri_handler(servidor_sta, &uri_on);
        httpd_register_uri_handler(servidor_sta, &uri_off);
        
        ESP_LOGI(TAG, "Servidor HTTP iniciado en modo STA");
    } else {
        ESP_LOGE(TAG, "Error al iniciar servidor HTTP en modo STA");
    }
}

/* ==================== Configuración WiFi - Modo AP ==================== */

static void inicializar_ap(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &manejador_eventos_wifi, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &manejador_eventos_wifi, NULL, NULL));

    wifi_config_t config_sta = {0};
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config_sta));

    wifi_config_t config_ap = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config_ap));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Punto de acceso configurado - SSID: %s", AP_SSID);
}

static void construir_pagina_escaneo(void) {
    wifi_scan_config_t config_escaneo = {0};
    ESP_ERROR_CHECK(esp_wifi_scan_start(&config_escaneo, true));

    uint16_t numero_aps = MAX_SCAN_LIST;
    wifi_ap_record_t lista_aps[MAX_SCAN_LIST];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&numero_aps, lista_aps));

    int bytes_usados = snprintf(buffer_html, sizeof(buffer_html),
        "<!DOCTYPE html>"
        "<html><head><title>Configuración WiFi - ESP32</title>"
        "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'></head>"
        "<body style='font-family: Arial; margin: 20px;'>"
        "<h2>Configuración de Red WiFi</h2>"
        "<p>Seleccione la red a la cual desea conectarse:</p>"
        "<form method='post' action='/conectar'>"
        "<div style='margin-bottom: 15px;'>"
        "<label>Red WiFi:</label><br>"
        "<select name='ssid' style='width: 100%%; padding: 8px; font-size: 16px;'>");

    for (int i = 0; i < numero_aps && bytes_usados < sizeof(buffer_html) - 500; i++) {
        const char* tipo_seguridad = (lista_aps[i].authmode == WIFI_AUTH_OPEN) ? " [Abierta]" : " [Segura]";
        bytes_usados += snprintf(buffer_html + bytes_usados, sizeof(buffer_html) - bytes_usados,
            "<option value='%s'>%s (%d dBm)%s</option>",
            (char*)lista_aps[i].ssid, (char*)lista_aps[i].ssid, 
            lista_aps[i].rssi, tipo_seguridad);
    }

    snprintf(buffer_html + bytes_usados, sizeof(buffer_html) - bytes_usados,
        "</select></div>"
        "<div style='margin-bottom: 15px;'>"
        "<label>Contraseña:</label><br>"
        "<input type='password' name='pass' style='width: 100%%; padding: 8px; font-size: 16px;' placeholder='Dejar vacío para redes abiertas'>"
        "</div>"
        "<button type='submit' style='width: 100%%; padding: 12px; font-size: 18px; background: #2196F3; color: white; border: none; border-radius: 4px;'>CONECTAR</button>"
        "</form>"
        "<hr><p style='font-size: 12px; color: #666;'>Redes detectadas: %d | IP del dispositivo: 192.168.4.1</p>"
        "</body></html>", numero_aps);
}

/* ==================== Configuración WiFi - Modo STA ==================== */

static void conectar_sta(const char *ssid, const char *password) {
    ESP_LOGI(TAG, "Iniciando conexión en modo estación");
    ESP_LOGI(TAG, "SSID objetivo: %s", ssid);

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);
    
    wifi_config_t config_wifi = {0};
    strncpy((char*)config_wifi.sta.ssid, ssid, sizeof(config_wifi.sta.ssid) - 1);
    strncpy((char*)config_wifi.sta.password, password, sizeof(config_wifi.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config_wifi));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void tarea_conexion_sta(void *parametros) {
    while(1) {
        if (solicitud_conexion_sta) {
            solicitud_conexion_sta = false;
            
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            if (servidor_ap) {
                ESP_LOGI(TAG, "Deteniendo servidor AP");
                httpd_stop(servidor_ap);
                servidor_ap = NULL;
            }
            
            vTaskDelay(pdMS_TO_TICKS(1000));
            conectar_sta(ssid_objetivo, password_objetivo);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ==================== Manejador de eventos WiFi ==================== */

static void manejador_eventos_wifi(void* arg, esp_event_base_t base, int32_t id, void* data) {
    static int contador_reintentos = 0;

    if (base == WIFI_EVENT && id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "Punto de acceso iniciado");
        construir_pagina_escaneo();
        iniciar_servidor_ap();
    } 
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Modo estación iniciado");
        esp_wifi_connect();
    } 
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (contador_reintentos < MAX_RETRY) {
            ESP_LOGI(TAG, "Reintento de conexión %d/%d", contador_reintentos + 1, MAX_RETRY);
            esp_wifi_connect();
            contador_reintentos++;
        } else {
            ESP_LOGE(TAG, "Conexión fallida después de %d intentos", MAX_RETRY);
            contador_reintentos = 0;
            ESP_LOGI(TAG, "Reiniciando sistema para volver al modo AP");
            vTaskDelay(pdMS_TO_TICKS(3000));
            esp_restart();
        }
    } 
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* evento = (ip_event_got_ip_t*) data;
        ESP_LOGI(TAG, "Conexión establecida exitosamente");
        ESP_LOGI(TAG, "Dirección IP asignada: " IPSTR, IP2STR(&evento->ip_info.ip));
        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&evento->ip_info.gw));
        ESP_LOGI(TAG, "Máscara de red: " IPSTR, IP2STR(&evento->ip_info.netmask));
        
        contador_reintentos = 0;
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        iniciar_servidor_sta();
        
        /* Indicación visual de conexión exitosa */
        for(int i = 0; i < 3; i++) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
}

/* ==================== Función principal ==================== */

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando sistema de aprovisionamiento WiFi");
    
    /* Inicialización de componentes del sistema */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* Creación de interfaces de red */
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    /* Creación de tarea para manejo de conexión STA */
    xTaskCreate(&tarea_conexion_sta, "tarea_conexion_sta", 4096, NULL, 5, NULL);

    /* Inicio del sistema en modo punto de acceso */
    inicializar_ap();
    
    ESP_LOGI(TAG, "Sistema iniciado correctamente");
    ESP_LOGI(TAG, "Instrucciones de uso:");
    ESP_LOGI(TAG, "1. Conectarse a la red: %s (contraseña: %s)", AP_SSID, AP_PASSWORD);
    ESP_LOGI(TAG, "2. Navegar a: http://192.168.4.1");
    ESP_LOGI(TAG, "3. Seleccionar red WiFi de destino y configurar");
}