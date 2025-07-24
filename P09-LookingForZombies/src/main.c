



/**
 * @file P09-LookingForZombies.c
 * @brief Griglio Tomás - Ejercicio 09: Creación de procesos y recolección de zombies.
 * @date 2025-07-23
 */
/**
    1- Escenario inicial: El padre duerme 40s, el hijo duerme 20s
    2- El hijo termina primero (después de 20s) 
    3- Cuando el padre despierta (después de 40s), llama a wait()
        wait() recolecta el estado del hijo terminado y limpia la entrada zombie
            Resultado: No hay procesos zombie
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Error al crear el proceso");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // ---------- Proceso Hijo ----------
        printf("[Hijo] PID: %d | PPID: %d\n", 
               getpid(), getppid());
        sleep(20);
        printf("[Hijo] Terminando después de 20 segundos.\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // ---------- Proceso Padre ----------
        printf("[Padre] PID: %d | Hijo creado: %d\n",
               getpid(), pid);
        sleep(40);
        printf("[Padre] Despertando, llamando wait()...\n");
        
        int status;
        pid_t child_pid = wait(&status);
        
        if (child_pid > 0) {
            printf("[Padre] Hijo %d recolectado exitosamente\n", child_pid);
            printf("[Padre] Estado del hijo: %d\n", status);
        }
        
        printf("[Padre] Terminando después de wait()\n");
        exit(EXIT_SUCCESS);
    }
}