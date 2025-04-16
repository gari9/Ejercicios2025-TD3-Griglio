#include "driver/gpio.h"
#include "button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "trafficLights.h"

#define PIN_BOTON GPIO_NUM_14                                  // Pin del botón

typedef enum { ESPERA, REBOTE, CONFIRMADO } estado_boton_t; // Estados del botón

static estado_boton_t estadoBoton = ESPERA;                   // Estado inicial del botón
static TickType_t tiempoRebote = 0;                           // Tiempo de rebote del botón inicializado a 0 (libreria de tiempo en ticks del FreeRTOS)

void iniciarBoton() {
    gpio_set_direction(PIN_BOTON, GPIO_MODE_INPUT);           // Configura el pin del botón como entrada
    gpio_set_pull_mode(PIN_BOTON, GPIO_PULLUP_ONLY);          // Habilita la resistencia pull-up interna
}

void actualizarBoton() { 
    bool lectura = !gpio_get_level(PIN_BOTON);                // Lee el estado del botón (inverso porque es pull-up)
    TickType_t ahora = xTaskGetTickCount();                   // Obtiene el tiempo actual del sistema

    switch (estadoBoton) {
        case ESPERA:                                          
            if (lectura) {                                    // Si el botón está presionado 
                estadoBoton = REBOTE;                       // Cambia al estado de rebote
                tiempoRebote = ahora;                         // Guarda el tiempo actual
            }
            break;
        case REBOTE:                                          // Estado de rebote
            if ((ahora - tiempoRebote) > pdMS_TO_TICKS(50)) {   // Espera 50 ms para el rebote 
                if (lectura) {                                  // Si el botón sigue presionado después del rebote
                    cambiarModo();                              // Cambia el modo del semáforo
                    estadoBoton = CONFIRMADO;
                } else {
                    estadoBoton = ESPERA;                       // Si no, vuelve al estado de espera
                }
            }
            break;
        case CONFIRMADO:                                        // Si el botón fue confirmado
            if (!lectura) {                                     // Si el botón se suelta
                estadoBoton = ESPERA;                           // Vuelve al estado de espera
            }
            break;
    }
}