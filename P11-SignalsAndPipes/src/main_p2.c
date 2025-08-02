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
#include <sys/types.h>
#include <string.h>

// Handler para la señal SIGCHLD
void sigchld_handler(int sig) {
    pid_t child_pid;
    int status;
    
    printf("Padre: Recibida señal SIGCHLD\n");
    
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
    int pipefd[2]; // pipefd[0] para lectura, pipefd[1] para escritura
    char buffer[256];
    ssize_t bytes_read;
    
    // Crear el pipe antes del fork
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
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
        
        // Cerrar el extremo de lectura del pipe (no lo necesita)
        close(pipefd[0]);
        
        printf("Hijo: Esperando 5 segundos antes de enviar mensaje...\n");
        sleep(5);
        
        // Enviar mensaje al padre
        const char *mensaje = "Hola padre! Soy el proceso hijo.";
        printf("Hijo: Enviando mensaje al padre...\n");
        
        if (write(pipefd[1], mensaje, strlen(mensaje) + 1) == -1) {
            perror("write");
            exit(1);
        }
        
        // Cerrar el extremo de escritura
        close(pipefd[1]);
        
        printf("Hijo: Esperando 10 segundos más antes de terminar...\n");
        sleep(10);
        
        printf("Hijo: Terminando...\n");
        exit(0);
        
    } else {
        // Proceso padre
        printf("Padre: Proceso hijo creado con PID = %d\n", pid);
        
        // Cerrar el extremo de escritura del pipe (no lo necesita)
        close(pipefd[1]);
        
        printf("Padre: Esperando mensaje del hijo...\n");
        
        // Leer mensaje del hijo
        bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Asegurar terminación null
            printf("Padre: Mensaje recibido del hijo: \"%s\"\n", buffer);
        } else if (bytes_read == 0) {
            printf("Padre: El hijo cerró el pipe sin enviar datos\n");
        } else {
            perror("read");
        }
        
        // Cerrar el extremo de lectura
        close(pipefd[0]);
        
        // Esperar más tiempo para que termine el hijo
        printf("Padre: Esperando a que termine el hijo...\n");
        sleep(12); // Dar tiempo suficiente (5 + 10 + margen)
        
        printf("Padre: Terminando programa\n");
    }
    
    return 0;
}