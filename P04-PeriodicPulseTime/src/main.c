/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//------------------ Inclusiones ------------------
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/gpio.h"
#include "pulsador.h"

//------------------ Constantes ---------------------
#define SALIDA1     GPIO_NUM_25
#define T_ESPERA_MS  40
#define T_ESPERA     pdMS_TO_TICKS(T_ESPERA_MS)
#define PROCESADORA 0
#define PROCESADORB 1
#define LED GPIO_NUM_2
#define PERIODO_MS 1000
#define TIEMPO_MINIMO pdMS_TO_TICKS(100)
//----------------- Prototipos ----------------------
void tareaLed( void* taskParmPtr ); //Prototipo de la función de la tarea
//----------------- Main ---------------------------
void app_main()
{
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    inicializarPulsador();

    // ---------- Creacion de la tarea -------------------
    // Creamos la tarea del LED 

    xTaskCreatePinnedToCore(
        tareaLed,
        "tareaLed",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL,
        1
    );

}

// Implementacion de funcion de la tarea

void tareaLed( void* taskParmPtr )
{
    // ---------- Configuraciones ------------------------------
    TickType_t tiempoOn = TIEMPO_MINIMO;
    const TickType_t periodo = pdMS_TO_TICKS(PERIODO_MS); 


    // ---------- Bucle infinito --------------------------
    while( true )
    {
        TickType_t nuevaDiferencia = obtenerDiferencia(); // Obtengo la diferencia de tiempo

        if(nuevaDiferencia != TIEMPO_NO_VALIDO && nuevaDiferencia < periodo )
        {
            tiempoOn = nuevaDiferencia; // Si la diferencia es valida y menor al periodo, la seteo como tiempoOn
            borrarDiferencia(); // Borro la diferencia
        }
        gpio_set_level( LED, 1 ); // Enciendo el led
        vTaskDelay( tiempoOn ); // Espero el tiempoOn
        gpio_set_level( LED, 0 ); // Apago el led
        vTaskDelay( periodo - tiempoOn ); // Espero el tiempo restante del periodo

    }
}
