#include "led.h"
#include "trafficLights.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_VERDE GPIO_NUM_25
#define LED_AMARILLO GPIO_NUM_26
#define LED_ROJO GPIO_NUM_27

#define T_VERDE     300
#define T_AMARILLO  100
#define T_ROJO      500
#define T_INTERMITENTE 300  // Tiempo de intermitente

typedef enum { MODO_NORMAL, MODO_INTERMITENTE } modo_t;                 // Modos de operación
typedef enum { ROJO, ROJO_AMARILLO, VERDE, SOLO_AMARILLO } estado_t;    // Estados del semáforo

static modo_t modoActual = MODO_NORMAL;                                 // Modo inicial
static estado_t estadoSemaforo = ROJO;                                  // Estado inicial del semáforo
static TickType_t tiempoAnterior = 0;                                   // Tiempo anterior para el semáforo

void iniciarSemaforo() {
    configurarLed(LED_ROJO, LED_AMARILLO, LED_VERDE);                   // Configura los LEDs
    tiempoAnterior = xTaskGetTickCount();                               // Inicializa el tiempo anterior
}

void cambiarModo() {
    if (modoActual == MODO_NORMAL) {
        modoActual = MODO_INTERMITENTE;
    } else {
        modoActual = MODO_NORMAL;
    }
      // Cambia el modo 
}

void actualizarSemaforo() {
    TickType_t ahora = xTaskGetTickCount();                              // Obtiene el tiempo actual del sistema
    if (modoActual == MODO_NORMAL) {                                     // Modo normal
        if ((ahora - tiempoAnterior) > pdMS_TO_TICKS(1000)) {
            tiempoAnterior = ahora;
            switch (estadoSemaforo) {
                case ROJO:
                    prenderLed('R'); apagarLed('A'); apagarLed('V');
                    estadoSemaforo = ROJO_AMARILLO;
                    break;
                case ROJO_AMARILLO:
                    prenderLed('R'); prenderLed('A'); apagarLed('V');
                    estadoSemaforo = VERDE;
                    break;
                case VERDE:
                    apagarLed('R'); apagarLed('A'); prenderLed('V');
                    estadoSemaforo = SOLO_AMARILLO;
                    break;
                case SOLO_AMARILLO:
                    apagarLed('R'); prenderLed('A'); apagarLed('V');
                    estadoSemaforo = ROJO;
                    break;
            }
        }
    } else if (modoActual == MODO_INTERMITENTE) {                       // Modo intermitente
        if ((ahora - tiempoAnterior) > pdMS_TO_TICKS(T_INTERMITENTE)) {
            tiempoAnterior = ahora;
            static bool encendido = false;
            encendido = !encendido;
            apagarLed('R'); 
            apagarLed('V');
            if (encendido) prenderLed('A'); else apagarLed('A');
        }
    }
}