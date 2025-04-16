#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "trafficLights.h"
#include "button.h"

void app_main() {
    iniciarSemaforo();
    iniciarBoton();

    while (1) {
        actualizarBoton();
        actualizarSemaforo();
        vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño delay para no saturar
    }
}