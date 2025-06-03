/*==================[ Inclusiones ]============================================*/
#include "pulsador.h"

/*==================[ Definiciones ]===================================*/

#define T_REBOTE_MS     40
#define T_REBOTE        pdMS_TO_TICKS(T_REBOTE_MS)
#define SALIDA_PRUEBA   GPIO_NUM_18

/*==================[Prototipos de funciones]======================*/

static void errorPulsador( uint8_t  indice );
static void botonPresionado( uint8_t  indice );
static void botonLiberado( uint8_t  indice );

void tareaPulsador( void* taskParmPtr );

/*==================[Variables]==============================*/
gpio_int_type_t pulsadorPines[N_PULSADOR] = { GPIO_NUM_36 };

pulsadorInfo pulsadorA;

/*==================[Implementaciones]=================================*/
TickType_t obtenerDiferencia(uint8_t  indice)
{
    TickType_t tiempo;
    tiempo = pulsadorA.diferenciaTiempo;
    return tiempo;
}

void borrarDiferencia( uint8_t  indice )
{
    pulsadorA.diferenciaTiempo = TIEMPO_NO_VALIDO;
}

void inicializarPulsador( void )
{
    pulsadorA.tecla             = pulsadorPines[0];
    pulsadorA.estado            = ALTO;                     //Estado inicial
    pulsadorA.tiempoBajo        = TIEMPO_NO_VALIDO;
    pulsadorA.tiempoAlto        = TIEMPO_NO_VALIDO;
    pulsadorA.diferenciaTiempo  = TIEMPO_NO_VALIDO;
    pulsadorA.cola = xQueueCreate( 1 , sizeof(eventoPulsador_t) );

    gpio_set_direction(pulsadorA.tecla , GPIO_MODE_INPUT);
    gpio_set_pull_mode(pulsadorA.tecla, GPIO_PULLDOWN_ONLY); //Habilita resistencia de PULLDOWN interna

    gpio_pad_select_gpio(SALIDA_PRUEBA);
    gpio_set_direction(SALIDA_PRUEBA, GPIO_MODE_OUTPUT);
 
    // Crear tareas en freeRTOS
    BaseType_t res = xTaskCreatePinnedToCore(
    	tareaPulsador,                     	// Funcion de la tarea a ejecutar
        "tareaPulsador",   	                // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
        NULL,                          	// Parametros de tarea
        tskIDLE_PRIORITY+1,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
        NULL,                          		// Puntero a la tarea creada en el sistema
        1
    );

    // Gestion de errores
	if(res == pdFAIL)
	{
		printf( "Error al crear la tarea.\r\n" );
		while(true);					// si no pudo crear la tarea queda en un bucle infinito
	}
}

static void errorPulsador( uint8_t  indice )
{
    pulsadorA.estado = ALTO;
}

// pulsador_ Update State Function
void actualizarPulsador( uint8_t  indice)
{
    switch( pulsadorA.estado )
    {
        case BAJO:
            if( gpio_get_level( pulsadorA.tecla ) ){
                pulsadorA.estado = ASCENDENTE;
            }
            break;

        case ASCENDENTE:
            if( gpio_get_level( pulsadorA.tecla ) ){
                pulsadorA.estado = ALTO;
                botonPresionado(indice);
            }
            else{
                pulsadorA.estado = BAJO;
            }
            break;

        case ALTO:
            if( !gpio_get_level( pulsadorA.tecla ) ){
                pulsadorA.estado = DESCENDENTE;
            }
            break;

        case DESCENDENTE:
            if( !gpio_get_level( pulsadorA.tecla ) ){
                pulsadorA.estado = BAJO;
                botonLiberado(indice);
            }
            else{
                pulsadorA.estado = ALTO;
            }
            break;

        default:
            errorPulsador(indice);
            break;
    }
}

/* accion de el evento de tecla pulsada */
static void botonPresionado( uint8_t  indice)
{
    TickType_t conteoTicksActuales = xTaskGetTickCount();   //Medimos el tiempo en ticks desde que inició el scheduler
    pulsadorA.tiempoBajo = conteoTicksActuales;             //guardamos ese tiempo como referencia
    gpio_set_level( SALIDA_PRUEBA, 1 );
}

/* accion de el evento de tecla liberada */
static void botonLiberado( uint8_t  indice )
{
    TickType_t conteoTicksActuales = xTaskGetTickCount();   // Medimos el tiempo en ticks desde que inició el scheduler
    pulsadorA.tiempoAlto = conteoTicksActuales;
    pulsadorA.diferenciaTiempo = pulsadorA.tiempoAlto - pulsadorA.tiempoBajo; // Tiempo en estado bajo
    gpio_set_level( SALIDA_PRUEBA, 0 );

    eventoPulsador_t evento;
    evento.indice = indice;
    evento.tiempo = pulsadorA.diferenciaTiempo;

    xQueueSend( pulsadorA.cola , &evento, portMAX_DELAY ); // Envío estructura a la cola
}


void tareaPulsador( void* taskParmPtr )
{
    while( true )
    {
        for (int i = 0; i < N_PULSADOR; i++)
        {
            actualizarPulsador(i);
        }
        vTaskDelay( T_REBOTE );
    }
}