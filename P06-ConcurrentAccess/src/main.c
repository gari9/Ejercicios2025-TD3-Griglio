#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/gpio.h"
#include "pulsador.h"

#define SALIDA1     GPIO_NUM_25
#define SALIDA2     GPIO_NUM_26
#define SALIDA3     GPIO_NUM_27
#define T_ESPERA_MS  40
#define T_ESPERA     pdMS_TO_TICKS(T_ESPERA_MS)
#define PROCESADORA 0
#define PROCESADORB 1
#define PERIODO_MS                    1000
#define PERIODO          pdMS_TO_TICKS(PERIODO_MS)

void tareaDestello( void* taskParmPtr ); //Prototipo de la función de la tarea
void tareaLedConstante( void* taskParmPtr ); //Prototipo de la función de la tarea
void tareaLedContador( void* taskParmPtr ); //Prototipo de la función de la tarea

void app_main()
{
    gpio_set_direction(SALIDA2, GPIO_MODE_OUTPUT);
    gpio_set_direction(SALIDA3, GPIO_MODE_OUTPUT);

    xTaskCreatePinnedToCore(tareaLedContador, "tareaLedContador", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL, PROCESADORA);
    xTaskCreatePinnedToCore(tareaLedConstante, "tareaLedConstante", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL, PROCESADORB);

    inicializarPulsador();

    BaseType_t res = xTaskCreatePinnedToCore(
    	tareaDestello,                     	// Funcion de la tarea a ejecutar
        "tareaDestello",   	                // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE, 		// Cantidad de stack de la tarea
        NULL,                          	// Parametros de tarea
        tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
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
    gpio_set_direction(SALIDA1, GPIO_MODE_OUTPUT);

    TickType_t dif;

    TickType_t xPeriodicity = PERIODO; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // ---------- Bucle infinito --------------------------
    while( true )
    {
        dif = obtenerDiferencia();

        if( dif > TIEMPO_NO_VALIDO )
        {
            if ( dif > PERIODO )
            {
                dif = PERIODO;
            }
            gpio_set_level( SALIDA1, 1 );
            vTaskDelay( dif );
            gpio_set_level( SALIDA1, 0 );

             vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
        }
        else
        {
            vTaskDelay( T_ESPERA );
        }

    }
}
void tareaLedContador(void* taskParmPtr)
{
    while (true)
    {
        uint32_t periodo = leerContador();
        gpio_set_level(SALIDA2, 1);
        vTaskDelay(pdMS_TO_TICKS(periodo / 2));
        gpio_set_level(SALIDA2, 0);
        vTaskDelay(pdMS_TO_TICKS(periodo / 2));
    }
}

void tareaLedConstante(void* taskParmPtr)
{
    const TickType_t periodo = pdMS_TO_TICKS(2000);

    while (true)
    {
        uint32_t valor = leerContador();
        gpio_set_level(SALIDA3, 1);
        vTaskDelay(pdMS_TO_TICKS(2 * valor));
        gpio_set_level(SALIDA3, 0);

        vTaskDelay(periodo - pdMS_TO_TICKS(2 * valor));
        decrementarContador();
    }
}
