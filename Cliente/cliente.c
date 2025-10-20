// cliente.c - Chat 1:1 con sockets + threads
// Compilar: gcc cliente.c -o cliente -pthread
// Ejecutar: ./cliente 127.0.0.1 5000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024

// Variable global: indica si el programa sigue corriendo
static volatile sig_atomic_t running = 1;

// Estructura para pasar datos a los threads
typedef struct {
    int sockfd;
} thread_args_t;

// Handler para Ctrl+C: pone running en 0 para terminar
static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

// Thread que recibe mensajes del servidor
// Se ejecuta en paralelo con send_thread
static void* recv_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char buf[BUF_SIZE];

    while (running) {
        // recv() espera datos del servidor (función bloqueante)
        ssize_t n = recv(args->sockfd, buf, sizeof(buf) - 1, 0);
        
        if (n < 0) {
            if (errno == EINTR) continue;  // Interrumpido por señal, reintentar
            perror("recv");
            break;
        } else if (n == 0) {
            // n == 0 significa que el servidor cerró la conexión
            printf("\n[Cliente] Servidor desconectado.\n");
            running = 0;
            break;
        }

        buf[n] = '\0';  // Agregar terminador de string
        printf("\nServidor: %s", buf);
        fflush(stdout);
    }

    return NULL;
}

// Thread que envía mensajes al servidor
// Lee desde el teclado y los manda por el socket
static void* send_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char line[BUF_SIZE];

    while (running) {
        // fgets() lee una línea del teclado (función bloqueante)
        if (!fgets(line, sizeof(line), stdin)) {
            // EOF en stdin (Ctrl+D)
            shutdown(args->sockfd, SHUT_WR);
            break;
        }
        
        // Comando para salir
        if (strncmp(line, "/quit", 5) == 0) {
            shutdown(args->sockfd, SHUT_WR);
            running = 0;
            break;
        }

        // send() envía los datos por el socket
        if (send(args->sockfd, line, strlen(line), 0) < 0) {
            perror("send");
            running = 0;
            break;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, handle_sigint);  // Manejar Ctrl+C

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // 1. Crear socket TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // 2. Configurar dirección del servidor
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)port);

    // Convertir IP de texto a formato binario
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 3. Connect: conectar al servidor (bloqueante)
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("[Cliente] Conectado a %s:%d\n", ip, port);
    printf("Escribí mensajes y presioná Enter. Comando: /quit\n\n");

    // 4. Crear threads para enviar y recibir en paralelo
    thread_args_t args = { .sockfd = sockfd };
    pthread_t th_recv, th_send;

    if (pthread_create(&th_recv, NULL, recv_thread, &args) != 0) {
        perror("pthread_create recv");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("pthread_create send");
        running = 0;
        pthread_join(th_recv, NULL);
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 5. Esperar a que terminen ambos threads
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    close(sockfd);
    printf("\n[Cliente] Cerrado.\n");
    
    return EXIT_SUCCESS;
}
