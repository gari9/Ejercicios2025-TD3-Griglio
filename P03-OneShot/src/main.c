/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */
//-------------------- Inclusiones -------------

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/gpio.h" //para driver de E/S digitales

//-------------------- Definiciones -------------
#define PULSADOR    GPIO_NUM_18
#define SALIDA1     GPIO_NUM_25
#define SALIDA2     GPIO_NUM_26
#define SALIDA3     GPIO_NUM_27
#define DELAY_MS    pdMS_TO_TICKS(10)

//-------------------- Prototipos -------------
void TaskA( void * pvParameters );
void TaskB( void * pvParameters );
void TaskC( void * pvParameters );
void TaskMonitor ( void * pvParameters );

//-------------------- Variables, constantes y punteros globales -------------
TaskHandle_t xHandleA = NULL;
TaskHandle_t xHandleB = NULL;
TaskHandle_t xHandleC = NULL;
TaskHandle_t xHandleMonitor = NULL;

//-------------------- app_main -------------
void app_main() 
{
    UBaseType_t prioridad = uxTaskPriorityGet(NULL);
    printf("Prioridad tarea principal: %d\n", prioridad);

    xTaskCreatePinnedToCore(TaskA, "Tarea A", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+3, &xHandleA, 0);
    configASSERT(xHandleA);

    xTaskCreatePinnedToCore(TaskB, "Tarea B", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+2, &xHandleB, 0);
    configASSERT(xHandleB);

    xTaskCreatePinnedToCore(TaskMonitor, "Tarea monitoreo", configMINIMAL_STACK_SIZE*3, NULL, tskIDLE_PRIORITY+1, &xHandleMonitor, 1);
    configASSERT(xHandleMonitor);
}

//-------------------- TaskA -------------
void TaskA( void * pvParameters )
{
    gpio_set_direction(PULSADOR, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PULSADOR, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(SALIDA1, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(SALIDA1, 1);
        vTaskDelay(DELAY_MS);
        gpio_set_level(SALIDA1, 0);
        vTaskDelay(DELAY_MS);

        if (gpio_get_level(PULSADOR))
        {
            if (xHandleC == NULL) // Solo crea TaskC si no existe
            {
                xTaskCreatePinnedToCore(TaskC, "Tarea C", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+4, &xHandleC, 1);
            }
        }
    }
}

//-------------------- TaskB -------------
void TaskB( void * pvParameters )
{
    gpio_set_direction(SALIDA2, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(SALIDA2, 1);
        vTaskDelay(DELAY_MS);
        gpio_set_level(SALIDA2, 0);
        vTaskDelay(DELAY_MS);
    }
}

//-------------------- TaskC -------------
void TaskC( void * pvParameters )
{
    gpio_set_direction(SALIDA3, GPIO_MODE_OUTPUT);

    gpio_set_level(SALIDA3, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(SALIDA3, 0);

    xHandleC = NULL; // Libera el handle global
    vTaskDelete(NULL); // Se autodestruye
}

//-------------------- TaskMonitor -------------
void TaskMonitor( void * pvParameters )
{
    while (true)
    {
        if (xHandleA != NULL)
            printf("Stack Tarea A libre: %u bytes\n", uxTaskGetStackHighWaterMark(xHandleA));
        if (xHandleB != NULL)
            printf("Stack Tarea B libre: %u bytes\n", uxTaskGetStackHighWaterMark(xHandleB));
        if (xHandleC != NULL)
            printf("Stack Tarea C libre: %u bytes\n", uxTaskGetStackHighWaterMark(xHandleC));
        else
            printf("Tarea C no existe\n");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
