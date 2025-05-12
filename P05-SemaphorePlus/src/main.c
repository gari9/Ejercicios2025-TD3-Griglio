#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/gpio.h"
#include "pulsador.h"

#define PERIODO_MS 1000
#define PERIODO pdMS_TO_TICKS(PERIODO_MS)
#define LED_VERDE GPIO_NUM_25
#define LED_ROJO  GPIO_NUM_26

extern pulsadorInfo pulsador[N_PULSADOR];

void tarea_led(void *taskParmPtr);

void app_main()
{
    inicializarPulsador(); // Crea tarea de lectura y FSM del pulsador

    gpio_set_direction(LED_VERDE, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ROJO, GPIO_MODE_OUTPUT);

    BaseType_t res = xTaskCreatePinnedToCore(
        tarea_led,
        "Tarea LED",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL,
        1
    );

    if (res == pdFAIL)
    {
        printf("Error al crear la tarea LED.\n");
        while (true);
    }
}

//---------------- Tarea B ------------------
static uint32_t contadorPulsaciones = 0;

void tarea_led(void *taskParmPtr)
{
    TickType_t xPeriodicity = PERIODO;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        if (xSemaphoreTake(pulsador[0].semaforo, 0) == pdTRUE)
        {
            gpio_set_level(LED_VERDE, 1);
            gpio_set_level(LED_ROJO, 0);
            contadorPulsaciones++;
            printf("Pulsaciones detectadas: %d\n", contadorPulsaciones);
        }
        else
        {
            gpio_set_level(LED_VERDE, 0);
            gpio_set_level(LED_ROJO, 1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));  // LED encendido durante 500ms

        // Apagar ambos LEDs después del destello
        gpio_set_level(LED_VERDE, 0);
        gpio_set_level(LED_ROJO, 0);


        vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
    }
}
