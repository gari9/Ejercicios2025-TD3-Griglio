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

int main()
{
    char buf[128];
    char operation[128];
    int s;
    float num1, num2;
    char op;
    int opcion;

    // Creamos socket PF_INET: IPV4, TCP
    s = socket(PF_INET,SOCK_STREAM, 0);

    // Cargamos datos de direccion de server
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

    int r = getaddrinfo("localhost","4096", &hints, &result); // NULL para localhost
    if (r != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        exit(EXIT_FAILURE);
    }
    // result es lista enlazada!. En este ejemplo usamos la primera

    // Ejecutamos connect()
    if (connect(s, (const struct sockaddr *)result->ai_addr, result->ai_addrlen) < 0)
    {
        fprintf(stderr,"ERROR connecting\r\n");
        close(s);
        exit(EXIT_FAILURE);
    }

    // Menú para el usuario
    printf("=== CALCULADORA TCP ===\n");
    printf("Seleccione una operación:\n");
    printf("1. Suma (+)\n");
    printf("2. Resta (-)\n");
    printf("3. Multiplicación (*)\n");
    printf("4. División (/)\n");
    printf("Opción: ");
    scanf("%d", &opcion);

    switch(opcion) {
        case 1: op = '+'; break;
        case 2: op = '-'; break;
        case 3: op = '*'; break;
        case 4: op = '/'; break;
        default:
            printf("Opción inválida\n");
            close(s);
            exit(EXIT_FAILURE);
    }

    printf("Ingrese el primer número: ");
    scanf("%f", &num1);
    printf("Ingrese el segundo número: ");
    scanf("%f", &num2);

    // Creamos el mensaje con el protocolo: "num1 operacion num2"
    sprintf(operation, "%.2f %c %.2f", num1, op, num2);
    printf("Enviando operación: %s\n", operation);

    // Enviamos mensaje a server
    int n = send(s, operation, strlen(operation), 0);
    if(n <= 0)
    {
        fprintf(stderr,"ERROR sending command\r\n");
        close(s);
        exit(EXIT_FAILURE);
    }

    // Leemos respuesta de server
    memset(buf, 0, sizeof(buf));
    n = recv(s, buf, sizeof(buf), 0);
    if(n > 0)
    {
        printf("Resultado: %s\n", buf);
    }
    else
    {
        fprintf(stderr,"ERROR receiving response\r\n");
    }

    freeaddrinfo(result);
    close(s);

    return 0;
}