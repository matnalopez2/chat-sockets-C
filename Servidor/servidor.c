// servidor.c - Chat 1:1 con sockets + threads
// Compilar: gcc servidor.c -o servidor -pthread
// Ejecutar: ./servidor 5000

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
// volatile = el compilador no optimiza lecturas/escrituras
// sig_atomic_t = tipo seguro para usar en signal handlers
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

// Thread que recibe mensajes del cliente
// Se ejecuta en paralelo con send_thread
static void* recv_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char buf[BUF_SIZE];

    while (running) {
        // recv() espera datos del cliente (función bloqueante)
        ssize_t n = recv(args->sockfd, buf, sizeof(buf) - 1, 0);
        
        if (n < 0) {
            if (errno == EINTR) continue;  // Interrumpido por señal, reintentar
            perror("recv");
            break;
        } else if (n == 0) {
            // n == 0 significa que el cliente cerró la conexión
            printf("\n[Servidor] Cliente desconectado.\n");
            running = 0;
            break;
        }

        buf[n] = '\0';  // Agregar terminador de string
        printf("\nCliente: %s", buf);
        fflush(stdout);
    }

    return NULL;
}

// Thread que envía mensajes al cliente
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
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, handle_sigint);  // Manejar Ctrl+C
    
    int port = atoi(argv[1]);
    
    // 1. Crear socket TCP
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Permitir reusar el puerto inmediatamente
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configurar dirección del servidor
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Escuchar en todas las interfaces
    addr.sin_port = htons((uint16_t)port);

    // 3. Bind: asociar socket con dirección y puerto
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return EXIT_FAILURE;
    }

    // 4. Listen: marcar socket como pasivo (acepta conexiones)
    if (listen(listenfd, 1) < 0) {
        perror("listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    printf("[Servidor] Esperando conexión en puerto %d...\n", port);

    // 5. Accept: aceptar conexión entrante (bloqueante)
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    
    if (connfd < 0) {
        perror("accept");
        close(listenfd);
        return EXIT_FAILURE;
    }

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliaddr.sin_addr, ipstr, sizeof(ipstr));
    printf("[Servidor] Cliente conectado: %s:%d\n", ipstr, ntohs(cliaddr.sin_port));
    printf("Escribí mensajes y presioná Enter. Comando: /quit\n\n");

    close(listenfd);  // Ya no necesitamos el socket de escucha

    // 6. Crear threads para enviar y recibir en paralelo
    thread_args_t args = { .sockfd = connfd };
    pthread_t th_recv, th_send;

    if (pthread_create(&th_recv, NULL, recv_thread, &args) != 0) {
        perror("pthread_create recv");
        close(connfd);
        return EXIT_FAILURE;
    }

    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("pthread_create send");
        running = 0;
        pthread_join(th_recv, NULL);
        close(connfd);
        return EXIT_FAILURE;
    }

    // 7. Esperar a que terminen ambos threads
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    close(connfd);
    printf("\n[Servidor] Cerrado.\n");

    return EXIT_SUCCESS;
}