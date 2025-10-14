// cliente.c - Chat 1:1 con sockets + threads (lado cliente)
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
static volatile sig_atomic_t running = 1;

typedef struct {
    int sockfd;
} thread_args_t;

static void handle_sigint (int sig){
    (void) sig;
    running = 0;
}


static void* recv_thread (void* arg) {
    thread_args_t* args = (thread_args_t*) arg;
    char buf[BUF_SIZE];

    while (running) {
        ssize_t n = recv(args->sockfd, buf, sizeof(buf)-1, 0);
        if(n < 0) {
            if (errno == EINTR) continue;
            perror("recv");
            break;
        } else if (n == 0) {
            printf("\n[Cliente] El servidor cerró la conexión.\n");
            running = 0;
            break;
        }

        buf[n] = '\0';
        printf("\nServidor: %s", buf);
        fflush(stdout);
    }

    return NULL;
}

static void* send_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char line[BUF_SIZE];

    while (running) {
        if (!fgets(line, sizeof(line), stdin)) {
            // EOF en stdin
            shutdown(args->sockfd, SHUT_WR);
            break;
        }
        if (strncmp(line, "/quit", 5) == 0) {
            shutdown(args->sockfd, SHUT_WR);
            running = 0;
            break;
        }
        size_t len = strlen(line);
        if (send(args->sockfd, line, len, 0) < 0) {
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

    signal(SIGINT, handle_sigint);

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return EXIT_FAILURE; }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("[Cliente] Conectado a %s:%d\n", ip, port);
    printf("Escribí mensajes y Enter para enviar. Comando: /quit para salir.\n");

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

    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    close(sockfd);
    printf("[Cliente] Cerrado.\n");
    return EXIT_SUCCESS;
}