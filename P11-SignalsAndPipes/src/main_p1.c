/**
 * @file main.c
 * @brief Ejemplo de comunicación entre procesos usando señales y pipes
 * @date 2025-07-28
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

// Handler para la señal SIGCHLD
void sigchld_handler(int sig) {
    pid_t child_pid;
    int status;
    
    printf("Padre: Recibida señal SIGCHLD\n");
    
    // Usar wait() para recoger el estado del proceso hijo
    child_pid = wait(&status);
    
    if (child_pid > 0) {
        printf("Padre: Proceso hijo %d terminó ", child_pid);
        if (WIFEXITED(status)) {
            printf("normalmente con código %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("por señal %d\n", WTERMSIG(status));
        }
    } else {
        perror("wait");
    }
}

int main() {
    pid_t pid;
    
    // Registrar el handler para SIGCHLD
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    
    printf("Padre: Creando proceso hijo...\n");
    printf("Padre: PID = %d\n", getpid());
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Proceso hijo
        printf("Hijo: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Hijo: Durmiendo por 5 segundos...\n");
        
        sleep(5);
        
        printf("Hijo: Terminando...\n");
        exit(42);   // Código de salida personalizado 
        
    } else {
        // Proceso padre
        printf("Padre: Proceso hijo creado con PID = %d\n", pid);
        printf("Padre: Esperando a que termine el hijo...\n");
        
        // El padre puede hacer otras tareas mientras espera
        // La señal SIGCHLD será manejada automáticamente
        sleep(10); // Dar tiempo suficiente para que termine el hijo
        
        printf("Padre: Terminando programa\n");
    }
    
    return 0;
}