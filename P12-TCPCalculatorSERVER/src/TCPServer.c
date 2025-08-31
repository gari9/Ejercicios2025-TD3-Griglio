#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

float calcular(float num1, char op, float num2) {
    switch(op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if(num2 == 0) {
                printf("Error: División por cero\n");
                return 0;
            }
            return num1 / num2;
        default:
            printf("Operación no válida\n");
            return 0;
    }
}

int main()
{
    socklen_t addr_len;
    struct sockaddr_in clientaddr;
    char buffer[128];
    char response[128];
    int newfd;
    int n;
    float num1, num2, resultado;
    char op;

    // Creamos socket
    int s = socket(PF_INET,SOCK_STREAM, 0);

    // Cargamos datos de IP:PORT del server
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

    int r = getaddrinfo(NULL,"4096", &hints, &result); // NULL para localhost
    if (r != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        exit(EXIT_FAILURE);
    }
    // result es lista enlazada!. En este ejemplo usamos la primera

    // Abrimos puerto con bind(), lo cual asigna una dirección al socket
    if (bind(s, (struct sockaddr*)result->ai_addr, result->ai_addrlen) == -1) {
        close(s);
        perror("listener: bind");
    }

    freeaddrinfo(result);

    // Seteamos socket en modo Listening
    if (listen (s, 10) == -1) // backlog=10, cant de pedidos que se almacenan
    {
        perror("error en listen");
        exit(1);
    }

    printf("Servidor calculadora iniciado en puerto 4096...\n");

    while(1)
    {
        // Ejecutamos accept() para recibir conexiones entrantes 
        addr_len = sizeof(struct sockaddr_in);
        if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
        {
            perror("error en accept");
            exit(1);
        }
        printf("server: conexion desde: %s\n", inet_ntoa(clientaddr.sin_addr));

        // Leemos mensaje de cliente
        memset(buffer, 0, sizeof(buffer));
        if( (n = recv(newfd,buffer,128,0)) == -1)
        {
            perror("Error leyendo mensaje en socket");
            exit(1);
        }
        buffer[n] = 0;
        printf("Recibi %d bytes: %s\n", n, buffer);

        // Parseamos el mensaje recibido (protocolo: "num1 op num2")
        if (sscanf(buffer, "%f %c %f", &num1, &op, &num2) == 3) {
            printf("Operación: %.2f %c %.2f\n", num1, op, num2);
            
            // Realizamos el cálculo
            resultado = calcular(num1, op, num2);
            
            // Preparamos la respuesta
            if (op == '/' && num2 == 0) {
                sprintf(response, "ERROR: División por cero");
            } else {
                sprintf(response, "%.2f", resultado);
            }
        } else {
            sprintf(response, "ERROR: Formato inválido");
        }

        printf("Enviando resultado: %s\n", response);

        // Enviamos mensaje a cliente
        if (send (newfd, response, strlen(response), 0) == -1)
        {
            perror("Error escribiendo mensaje en socket");
            exit (1);
        }

        // Cerramos conexion con cliente
        close(newfd);
    }

    return 0;
}