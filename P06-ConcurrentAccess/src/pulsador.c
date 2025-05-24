/*==================[ Inclusiones ]============================================*/
#include "pulsador.h"

/*==================[ Definiciones ]===================================*/

#define T_REBOTE_MS   40
#define T_REBOTE pdMS_TO_TICKS(T_REBOTE_MS)
#define SALIDA_PRUEBA   GPIO_NUM_26

/*==================[Prototipos de funciones]======================*/

static void errorPulsador( void );
static void botonPresionado( void );
static void botonLiberado( void );

void tareaPulsador( void* taskParmPtr );

/*==================[Variables]==============================*/
//gpio_int_type_t pulsadorPines[1] = { GPIO_NUM_18 };
gpio_int_type_t pulsadorPines[2] = { GPIO_NUM_18, GPIO_NUM_19 };
pulsadorInfo pulsadorA;
pulsadorInfo pulsadorB;

static uint32_t contador = 500; 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado

/*==================[Implementaciones]=================================*/

// FUNCIONES DE ACCESO PROTEGIDO AL CONTADOR
void incrementarContador(void) {
    portENTER_CRITICAL(&mux);
    if (contador < 900) contador += 100;
    portEXIT_CRITICAL(&mux);
}

void decrementarContador(void) {
    portENTER_CRITICAL(&mux);
    if (contador > 100) contador -= 100;
    portEXIT_CRITICAL(&mux);
}

uint32_t leerContador(void) {
    uint32_t valor;
    portENTER_CRITICAL(&mux);
    valor = contador;
    portEXIT_CRITICAL(&mux);
    return valor;
}
TickType_t obtenerDiferencia()
{
    TickType_t tiempo;
    portENTER_CRITICAL(&mux);
    tiempo = pulsadorA.diferenciaTiempo;
    portEXIT_CRITICAL(&mux);
    return tiempo;
}

void borrarDiferencia( void )
{
    portENTER_CRITICAL(&mux);
    pulsadorA.diferenciaTiempo = TIEMPO_NO_VALIDO;
    portEXIT_CRITICAL(&mux);
}

void inicializarPulsador( void )
{
    // Configuracion de primer pulsador

    pulsadorA.tecla             = pulsadorPines[0];
    pulsadorA.estado            = ALTO;                     //Estado inicial
    pulsadorA.tiempoBajo        = TIEMPO_NO_VALIDO;
    pulsadorA.tiempoAlto        = TIEMPO_NO_VALIDO;
    pulsadorA.diferenciaTiempo  = TIEMPO_NO_VALIDO;

    // Configuracion de segundo pulsador 

    pulsadorB.tecla             = pulsadorPines[1];
    pulsadorB.estado            = ALTO;
    pulsadorB.tiempoBajo        = TIEMPO_NO_VALIDO;
    pulsadorB.tiempoAlto        = TIEMPO_NO_VALIDO;
    pulsadorB.diferenciaTiempo  = TIEMPO_NO_VALIDO;

    gpio_set_direction(pulsadorB.tecla , GPIO_MODE_INPUT);
    gpio_set_pull_mode(pulsadorB.tecla, GPIO_PULLDOWN_ONLY);


    gpio_set_direction(pulsadorA.tecla , GPIO_MODE_INPUT);
    gpio_set_pull_mode(pulsadorA.tecla, GPIO_PULLDOWN_ONLY); //Habilita resistencia de PULLDOWN interna

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

static void errorPulsador( void )
{
    portENTER_CRITICAL(&mux);
    pulsadorA.estado = ALTO;
    portEXIT_CRITICAL(&mux);
}

// pulsador_ Update State Function
void actualizarPulsador(pulsadorInfo *pulsador, void (*accion)(void))
{
    switch( pulsador->estado )
    {
        case BAJO:
            if( gpio_get_level( pulsador->tecla ) ){
                pulsador->estado = ASCENDENTE;
            }
            break;

        case ASCENDENTE:
            if( gpio_get_level( pulsador->tecla ) ){
                pulsador->estado = ALTO;
                accion(); // Ejecuta acción asignada
            }
            else{
                pulsador->estado = BAJO;
            }
            break;

        case ALTO:
            if( !gpio_get_level( pulsador->tecla ) ){
                pulsador->estado = DESCENDENTE;
            }
            break;

        case DESCENDENTE:
            if( !gpio_get_level( pulsador->tecla ) ){
                pulsador->estado = BAJO;
            }
            else{
                pulsador->estado = ALTO;
            }
            break;

        default:
            errorPulsador();
            break;
    }
}

/* accion de el evento de tecla pulsada */
static void botonPresionado()
{
    TickType_t conteoTicksActuales = xTaskGetTickCount();   //Medimos el tiempo en ticks desde que inició el scheduler
    gpio_set_level( SALIDA_PRUEBA, 1 );         //para tener una referencia en el debug
    pulsadorA.tiempoBajo = conteoTicksActuales;             //guardamos ese tiempo como referencia
}

/* accion de el evento de tecla liberada */
static void botonLiberado()
{
    TickType_t conteoTicksActuales = xTaskGetTickCount();   //Medimos el tiempo en ticks desde que inició el scheduler
    gpio_set_level( SALIDA_PRUEBA, 0 );         //para tener una referencia en el debug
    pulsadorA.tiempoAlto    = conteoTicksActuales;
    portENTER_CRITICAL(&mux);
    pulsadorA.diferenciaTiempo  = pulsadorA.tiempoAlto - pulsadorA.tiempoBajo; //Da el tiempo que el pulsador estuvo en estado alto
    portEXIT_CRITICAL(&mux);
}

void tareaPulsador( void* taskParmPtr )
{
    while( true )
    {
        actualizarPulsador(&pulsadorA, incrementarContador);
        actualizarPulsador(&pulsadorB, decrementarContador);
        vTaskDelay( T_REBOTE );
    }
}
