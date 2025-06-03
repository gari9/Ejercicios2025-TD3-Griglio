#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/gpio.h"
#include "pulsador.h"

#define SALIDA1     GPIO_NUM_25
#define PERIODO_MS  1000
#define PERIODO     pdMS_TO_TICKS(PERIODO_MS)
#define PROCESADORA 0
#define PROCESADORB 1
#define N_LED       1

gpio_int_type_t led [N_PULSADOR] = {GPIO_NUM_16};

extern pulsadorInfo pulsadorA;

void tareaDestello( void* taskParmPtr ); //Prototipo de la función de la tarea

void app_main()
{
    // Crear tarea en freeRTOS
    // Devuelve pdPASS si la tarea fue creada y agregada a la lista ready
    // En caso contrario devuelve pdFAIL.
    inicializarPulsador();

    BaseType_t res = xTaskCreatePinnedToCore(
            tareaDestello,                     	// Funcion de la tarea a ejecutar
            "tareaDestello",   	                // Nombre de la tarea como String amigable para el usuario
            configMINIMAL_STACK_SIZE*3, 		// Cantidad de stack de la tarea
            NULL,                          	// Parametros de tarea
            tskIDLE_PRIORITY,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
            NULL,                          		// Puntero a la tarea creada en el sistema
            PROCESADORA
        );
    // Gestion de errores
    if(res == pdFAIL)
    {
        printf( "Error al crear la tarea.\r\n" );
        while(true);					// si no pudo crear la tarea queda en un bucle infinito
    }
}

// Implementacion de funcion de la tarea
void tareaDestello( void* taskParmPtr )
{
    // ---------- Configuraciones ------------------------------
    gpio_set_direction(led[0], GPIO_MODE_OUTPUT);

    TickType_t xPeriodicity =  PERIODO;
	TickType_t xLastWakeTime = xTaskGetTickCount();

    eventoPulsador_t evento;

    // ---------- Bucle infinito --------------------------
    while( true )
    {
        if(xQueueReceive(pulsadorA.cola, &evento, 0))
{
            TickType_t dif = evento.tiempo;
                if( dif > xPeriodicity )
                    {
                        dif = xPeriodicity;
                    }
            printf("LED %d, Tiempo medido %u\n", evento.indice, dif); // para debug
        }
        else
        {
        gpio_set_level( led[0], 1 );
        vTaskDelay( dif );
        gpio_set_level( led[0], 0 );

        vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
        }
}