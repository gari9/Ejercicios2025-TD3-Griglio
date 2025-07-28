/**
 * @file P10-OpenDoors.c
 * @brief Sistema escalable de control de puertas usando hilos (pthreads)
 * @date 2025-07-23
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// Estructura para  puerta
typedef struct {
    char letra;
    char nombre[20];
    bool abierta;
    pthread_t thread_id;
    pthread_mutex_t mutex;
} Puerta;

// Array escalable de puertas
Puerta puertas[10];  // Soporta hasta 10 puertas
int num_puertas = 3; // Inciamos con A, B, C

// Mutex global para sincronización de consola
pthread_mutex_t mutex_consola = PTHREAD_MUTEX_INITIALIZER;

// Función para manejar cualquier puerta
void* manejar_puerta(void* arg) {
    Puerta* puerta = (Puerta*)arg;
    char input;
    
    printf("[Hilo %ld] Iniciado para %s\n", 
           pthread_self(), puerta->nombre);
    
    while (1) {
        // Esperar entrada del usuario
        if (scanf(" %c", &input) == 1) {
            if (input == puerta->letra || input == puerta->letra + 32) {    // Soporta mayúsculas y minúsculas
                // Bloquear mutex de la puerta
                pthread_mutex_lock(&puerta->mutex);
                
                if (!puerta->abierta) {
                    // Abrir puerta
                    puerta->abierta = true;
                    pthread_mutex_lock(&mutex_consola);
                    printf("[Hilo %ld] %s abierta, presione %c para cerrarla\n",
                           pthread_self(), puerta->nombre, puerta->letra);
                    pthread_mutex_unlock(&mutex_consola);
                } else {
                    // Cerrar puerta
                    puerta->abierta = false;
                    pthread_mutex_lock(&mutex_consola);
                    printf("[Hilo %ld] %s cerrada, presione %c para abrirla\n",
                           pthread_self(), puerta->nombre, puerta->letra);
                    pthread_mutex_unlock(&mutex_consola);
                }
                
                pthread_mutex_unlock(&puerta->mutex);
                
                // Simular tiempo de apertura/cierre
                usleep(500000); 
            }
        }
        
        // Permitir cancelación del hilo
        pthread_testcancel();
    }
    
    return NULL;
}

// Función para agregar una nueva puerta (escalabilidad)
int agregar_puerta(char letra) {
    if (num_puertas >= 10) {
        printf("Error: Máximo 10 puertas soportadas\n");
        return -1;
    }
    
    Puerta* nueva_puerta = &puertas[num_puertas];
    nueva_puerta->letra = letra;
    sprintf(nueva_puerta->nombre, "Puerta %c", letra);
    nueva_puerta->abierta = false;
    
    // Inicializar mutex
    pthread_mutex_init(&nueva_puerta->mutex, NULL);
    
    // Crear hilo
    if (pthread_create(&nueva_puerta->thread_id, NULL, 
                      manejar_puerta, nueva_puerta) != 0) {
        printf("Error al crear hilo para %s\n", nueva_puerta->nombre);
        return -1;
    }
    
    printf("✓ %s agregada con hilo %ld\n", 
           nueva_puerta->nombre, nueva_puerta->thread_id);
    
    num_puertas++;
    return 0;
}

int main() {
    printf("=== Sistema P10-OpenDoors (Hilos) ===\n");
    printf("Inicializando puertas...\n");
    
    // Inicializar puertas A, B, C
    char letras[] = {'A', 'B', 'C'};
    for (int i = 0; i < 3; i++) {
        puertas[i].letra = letras[i];
        sprintf(puertas[i].nombre, "Puerta %c", letras[i]);
        puertas[i].abierta = false;
        pthread_mutex_init(&puertas[i].mutex, NULL);
        
        if (pthread_create(&puertas[i].thread_id, NULL, 
                          manejar_puerta, &puertas[i]) != 0) {
            printf("Error al crear hilo para Puerta %c\n", letras[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    printf("✓ Sistema iniciado con %d puertas\n", num_puertas);
    printf("Presione A, B, C para controlar puertas\n");
    printf("(El sistema es escalable - se pueden agregar más puertas)\n");
    

    
    // Esperar que terminen todos los hilos
    for (int i = 0; i < num_puertas; i++) {
        pthread_join(puertas[i].thread_id, NULL);
        pthread_mutex_destroy(&puertas[i].mutex);
    }
    
    pthread_mutex_destroy(&mutex_consola);
    return 0;
}