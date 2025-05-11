#ifndef PULSADOR_H_
#define PULSADOR_H_
/*==================[ Inclusiones ]============================================*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
/*==================[ Definiciones ]===================================*/
#define TIEMPO_NO_VALIDO   0

typedef enum
{
    ALTO,
    BAJO,
    DESCENDENTE,
    ASCENDENTE
} estadoPulsador;

typedef struct
{
    gpio_int_type_t tecla; //pin del pulsador
    estadoPulsador estado;   //variables
    TickType_t tiempoBajo;		//tiempo de la última transición del estado alto a bajo
    TickType_t tiempoAlto;		    //tiempo de la última transición del estado bajo a alto
    TickType_t diferenciaTiempo;	    //variables
} pulsadorInfo;

/*==================[Prototipos de funciones]======================*/
void inicializarPulsador( void );

/**
 * @brief Devuelvo el tiempo de pulsación entre flanco ascendente y descendente
 * 
 * @return TickType_t El tiempo en Ticks
 */
TickType_t obtenerDiferencia( void);

/**
 * @brief Vuelve a 0 el tiempo de pulsación
 * 
 */
void borrarDiferencia( void );

#endif