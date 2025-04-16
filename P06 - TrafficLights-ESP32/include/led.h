/**
 * @file led.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef LED_H //chequea que otro archivo no haya llamado a led.h
#define LED_H

#include "driver/gpio.h"

/**
 * @brief 
 * 
 * @param ledR Pin donde está conectado el led rojo
 * @param ledA Pin donde está conectado el led amarillo
 * @param ledV Pin donde está conectado el led verde
 */
void configurarLed(gpio_int_type_t ledR, gpio_int_type_t ledA, gpio_int_type_t ledV); //especifica en que pines conectar cada led: ROJO, AMARILLO, VERDE

/**
 * @brief Función para encender un led
 * 
 * @param led Led a encender R para rojo, V para verde A para amarillo
 */
void prenderLed(char led);

/**
 * @brief Función para apagar un led
 * 
 * @param led Led a apagar R para rojo, V para verde A para amarillo
 */
void apagarLed(char led);

#endif