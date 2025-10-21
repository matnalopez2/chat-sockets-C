// ============================================================================
// servidor.c - Servidor de chat simple usando sockets
// ============================================================================
// Compilar: gcc servidor.c ../util/network.c -o servidor -I../util
// Ejecutar: ./servidor 5000
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    // Verificar argumentos
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        printf("Ejemplo: %s 5000\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int sockfd;
    char buffer[BUF_SIZE] = {0};
    
    printf("\n=== SERVIDOR DE CHAT ===\n");
    printf("Puerto: %d\n", port);
    printf("Esperando cliente...\n\n");

    // Abrir el servidor (crea socket, bind, listen y accept)
    sockfd = OpenServer(port);
    if (sockfd <= 0) {
        printf("Error: No se pudo iniciar el servidor\n");
        return EXIT_FAILURE;
    }
    
    printf("Cliente conectado!\n");
    printf("Escribe '/quit' para salir\n");
    printf("─────────────────────────────\n\n");

    // Loop principal del chat
    while (1) {
        // 1. RECIBIR mensaje del cliente
        memset(buffer, 0, BUF_SIZE);  // Limpiar buffer
        int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
        
        // Verificar si el cliente se desconectó
        if (bytes <= 0) {
            printf("\nCliente desconectado.\n");
            break;
        }
        
        buffer[bytes] = '\0';  // Asegurar terminación
        
        // Verificar si el cliente envió /quit
        if (strncmp(buffer, "/quit", 5) == 0) {
            printf("\nCliente cerró la conexión.\n");
            break;
        }
        
        // Mostrar mensaje recibido
        printf("Cliente: %s", buffer);
        if (buffer[bytes-1] != '\n') printf("\n");
        
        // 2. ENVIAR respuesta al cliente
        printf("Tú: ");
        fflush(stdout);
        
        // Leer mensaje del usuario
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            break;  // EOF o error
        }
        
        // Verificar si el servidor quiere salir
        if (strncmp(buffer, "/quit", 5) == 0) {
            printf("Cerrando conexión...\n");
            send(sockfd, buffer, strlen(buffer), 0);
            break;
        }
        
        // Enviar mensaje al cliente
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            printf("Error al enviar mensaje\n");
            break;
        }
    }

    // Cerrar conexión
    CloseServer(sockfd);
    printf("\nServidor cerrado.\n\n");
    
    return EXIT_SUCCESS;
}
