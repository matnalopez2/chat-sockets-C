// servidor.c - Chat 1:1 con sockets + threads (lado servidor)
// Compilar: gcc servidor.c -o servidor -pthread
// Ejecutar: ./servidor 5000
// Luego conectar con el cliente a la IP del servidor y ese puerto.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


#define BUF_SIZE 1024

static volatile sig_atomic_t running = 1;   // bandera global para terminar ordenadamente

typedef struct {
    int sockfd;
} thread_args_t;

static void handle_sigint (int sig){
    (void) sig;
    running = 0;
}

static void* recv_thread (void* arg){
    thread_args_t* args = (thread_args_t*) arg;
    char buf[BUF_SIZE];

    while (running) {
        ssize_t n = recv(args->sockfd, buf, sizeof(buf)-1, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recv");
            break;
        } else if (n == 0){
            // el peer cerró la conexión
            printf("\n[Servidor] El cliente cerró la conexión.\n");
            running = 0;
            break;
        }

        buf[n] = '\0';
        printf("\nCliente: %s", buf);
        fflush(stdout);
    }

    return NULL;
}

static void* send_thread (void* arg){
    thread_args_t* args = (thread_args_t*) arg;
    char line[BUF_SIZE];

    while (running) {
        if (!fgets(line, sizeof(line), stdin)) {
            //EOF en stdin -> cortar envío half-close
            shutdown(args->sockfd, SHUT_WR);
            break;
        }
        if(strncmp(line, "/quit", 5) == 0 ) {
            shutdown(args->sockfd, SHUT_WR);
            running = 0;
            break;
        }
        size_t len = strlen(line);
        if(send(args->sockfd, line, len, 0) < 0 ){
            perror("send");
            running = 0;
            break;
        }
    }
    return NULL;
}

int main(int argc, char* argv[] ){
    if (argc != 2){
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, handle_sigint);

    int port = atoi(argv[1]);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0 ) {
        perror("socket");
        return EXIT_FAILURE;
    }

    //Reusar puerto rápido tras reiniciar el servidor
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //escucha en todas las interfaces
    addr.sin_port = htons((uint16_t) port);

    if(bind(listenfd, (struct sockaddr*) &addr, sizeof(addr)) < 0 ){
        perror("bind");
        close(listenfd);
        return EXIT_FAILURE;
    }

    if(listen(listenfd, 1) < 0 ) {
        perror("listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    printf("[Servidor] Esperando conexión en el puerto %d...\n", port);

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
    if(connfd < 0) {
        perror("accept");
        close(listenfd);
        return EXIT_FAILURE;
    }

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliaddr.sin_addr, ipstr, sizeof(ipstr));
    printf("[Servidor] Conectado con %s: %d\n", ipstr, ntohs(cliaddr.sin_port));
    printf("Escribí mensajes y Enter para enviar. Comando /quit para salir.\n");

    thread_args_t args = { .sockfd = connfd };
    pthread_t th_recv, th_send;

    if(pthread_create(&th_recv, NULL, recv_thread, &args) != 0 ){
        perror("pthread_create recv");
        close(connfd);
        close(listenfd);
        return EXIT_FAILURE;
    }

    if(pthread_create(&th_send, NULL, send_thread, &args) != 0 ){
        perror("pthread_create send");
        running = 0;
        pthread_join(th_recv, NULL);
        close(connfd);
        close(listenfd);
        return EXIT_FAILURE;
    }

    //Esperar a que terminen los hilos
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    close(connfd);
    close(listenfd);
    printf("[Servidor] Cerrado.\n");

    return EXIT_SUCCESS;
}