// ============================================================================
// servidor.c - Chat 1:1 con sockets + threads (lado servidor)
// ============================================================================
// Ejemplo educativo de programación con sockets TCP y threads en C
// 
// Compilar: gcc servidor.c -o servidor -pthread
// Ejecutar: ./servidor 5000
// 
// Este programa demuestra:
//   - Uso de sockets TCP (socket, bind, listen, accept)
//   - Programación con threads POSIX (pthread)
//   - Sincronización con mutex
//   - Manejo de señales (SIGINT/Ctrl+C)
//   - I/O concurrente (lectura/escritura simultánea)
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

// ============================================================================
// CONSTANTES Y DEFINICIONES
// ============================================================================

#define BUF_SIZE 1024          // Tamaño del buffer para mensajes
#define MIN_PORT 1024          // Puerto mínimo permitido (puertos > 1023 no requieren root)
#define MAX_PORT 65535         // Puerto máximo permitido
#define BACKLOG 1              // Número de conexiones pendientes en cola

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

// Bandera global para terminar ordenadamente (acceso protegido por mutex)
static int running = 1;

// Mutex para proteger la variable 'running' y evitar race conditions
static pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;

// ============================================================================
// ESTRUCTURA PARA PASAR DATOS A LOS THREADS
// ============================================================================

typedef struct {
    int sockfd;              // File descriptor del socket conectado
    pthread_mutex_t* mutex;  // Puntero al mutex compartido
} thread_args_t;

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

/**
 * Verifica si el programa debe seguir ejecutándose
 * Thread-safe: usa mutex para acceso seguro
 * 
 * @return 1 si debe continuar, 0 si debe terminar
 */
static int is_running(void) {
    pthread_mutex_lock(&running_mutex);
    int val = running;
    pthread_mutex_unlock(&running_mutex);
    return val;
}

/**
 * Establece el estado de ejecución del programa
 * Thread-safe: usa mutex para acceso seguro
 * 
 * @param val: 0 para detener, 1 para continuar
 */
static void set_running(int val) {
    pthread_mutex_lock(&running_mutex);
    running = val;
    pthread_mutex_unlock(&running_mutex);
}

/**
 * Manejador de señal para SIGINT (Ctrl+C)
 * Permite cerrar el programa ordenadamente
 * 
 * @param sig: número de señal recibida
 */
static void handle_sigint(int sig) {
    (void)sig;  // Evitar warning de parámetro no usado
    printf("\n[Servidor] Señal de interrupción recibida. Cerrando...\n");
    set_running(0);
}

// ============================================================================
// THREAD DE RECEPCIÓN
// ============================================================================

/**
 * Thread que recibe mensajes del cliente
 * Se ejecuta concurrentemente con el thread de envío
 * 
 * @param arg: puntero a thread_args_t con socket y mutex
 * @return NULL al terminar
 */
static void* recv_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char buf[BUF_SIZE];

    while (is_running()) {
        // recv() bloqueante: espera datos del cliente
        // Dejamos un byte para el '\0' terminal
        ssize_t n = recv(args->sockfd, buf, sizeof(buf) - 1, 0);
        
        if (n < 0) {
            // Error en recv
            if (errno == EINTR) {
                // Señal interrumpió recv, reintentamos
                continue;
            }
            if (is_running()) {
                // Solo mostramos error si no estamos cerrando
                perror("[Servidor] Error en recv");
            }
            break;
        } else if (n == 0) {
            // n == 0 significa que el peer cerró la conexión ordenadamente
            printf("\n[Servidor] El cliente cerró la conexión.\n");
            set_running(0);
            break;
        }

        // Agregar terminador nulo y mostrar mensaje
        buf[n] = '\0';
        
        // Imprimir con formato para mejor visualización
        printf("\n┌─[Cliente]────────────────────────────────────────────────┐\n");
        printf("│ %s", buf);
        if (buf[n-1] != '\n') {
            printf("\n");
        }
        printf("└──────────────────────────────────────────────────────────┘\n");
        printf("[Tú] > ");
        fflush(stdout);
    }

    return NULL;
}

// ============================================================================
// THREAD DE ENVÍO
// ============================================================================

/**
 * Thread que envía mensajes al cliente
 * Lee de stdin y envía por el socket
 * 
 * @param arg: puntero a thread_args_t con socket y mutex
 * @return NULL al terminar
 */
static void* send_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char line[BUF_SIZE];

    printf("[Tú] > ");
    fflush(stdout);

    while (is_running()) {
        // fgets() bloqueante: lee una línea desde stdin
        if (!fgets(line, sizeof(line), stdin)) {
            // EOF en stdin (por ejemplo, Ctrl+D)
            // Usamos shutdown para cerrar la escritura (half-close)
            printf("\n[Servidor] EOF detectado en stdin.\n");
            shutdown(args->sockfd, SHUT_WR);
            set_running(0);
            break;
        }

        // Comando especial para salir
        if (strncmp(line, "/quit", 5) == 0) {
            printf("[Servidor] Comando /quit recibido. Cerrando...\n");
            shutdown(args->sockfd, SHUT_WR);
            set_running(0);
            break;
        }

        // Enviar el mensaje al cliente
        size_t len = strlen(line);
        ssize_t sent = send(args->sockfd, line, len, 0);
        
        if (sent < 0) {
            perror("[Servidor] Error en send");
            set_running(0);
            break;
        }

        // Mostrar prompt para el siguiente mensaje
        // (puede ser interrumpido por mensajes entrantes)
        if (is_running()) {
            printf("[Tú] > ");
            fflush(stdout);
        }
    }
    
    return NULL;
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

/**
 * Valida que el puerto esté en rango válido
 * 
 * @param port: número de puerto a validar
 * @return 1 si es válido, 0 si no
 */
static int validate_port(int port) {
    return (port >= MIN_PORT && port <= MAX_PORT);
}

int main(int argc, char* argv[]) {
    // ========================================================================
    // 1. VALIDACIÓN DE ARGUMENTOS
    // ========================================================================
    
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        fprintf(stderr, "  puerto: número entre %d y %d\n", MIN_PORT, MAX_PORT);
        fprintf(stderr, "Ejemplo: %s 5000\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if (!validate_port(port)) {
        fprintf(stderr, "[Error] Puerto inválido: %d\n", port);
        fprintf(stderr, "El puerto debe estar entre %d y %d\n", MIN_PORT, MAX_PORT);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 2. CONFIGURACIÓN DE SEÑALES
    // ========================================================================
    
    // Capturar Ctrl+C para cierre ordenado
    signal(SIGINT, handle_sigint);
    
    // Ignorar SIGPIPE (se genera al escribir en socket cerrado)
    // Preferimos manejar el error en send() en lugar de recibir señal
    signal(SIGPIPE, SIG_IGN);

    // ========================================================================
    // 3. CREACIÓN DEL SOCKET
    // ========================================================================
    
    // socket() crea un endpoint de comunicación
    // AF_INET: IPv4, SOCK_STREAM: TCP, 0: protocolo por defecto
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("[Error] socket");
        return EXIT_FAILURE;
    }

    // Configurar opción SO_REUSEADDR para reutilizar puerto rápidamente
    // Útil al reiniciar el servidor sin esperar TIME_WAIT
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[Advertencia] setsockopt SO_REUSEADDR");
        // No es crítico, continuamos
    }

    // ========================================================================
    // 4. BIND: ASOCIAR SOCKET CON DIRECCIÓN Y PUERTO
    // ========================================================================
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Escucha en todas las interfaces
    addr.sin_port = htons((uint16_t)port);     // htons: host to network short

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[Error] bind");
        fprintf(stderr, "¿El puerto %d ya está en uso?\n", port);
        close(listenfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 5. LISTEN: MARCAR SOCKET COMO PASIVO (ACEPTA CONEXIONES)
    // ========================================================================
    
    if (listen(listenfd, BACKLOG) < 0) {
        perror("[Error] listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    printf("═══════════════════════════════════════════════════════════\n");
    printf("  SERVIDOR DE CHAT - Modo Escucha\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Puerto: %d\n", port);
    printf("  Estado: Esperando conexión de cliente...\n");
    printf("  Presiona Ctrl+C para terminar\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    // ========================================================================
    // 6. ACCEPT: ACEPTAR CONEXIÓN ENTRANTE (BLOQUEANTE)
    // ========================================================================
    
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    
    if (connfd < 0) {
        if (errno == EINTR) {
            // Interrumpido por señal (probablemente Ctrl+C)
            printf("\n[Servidor] Accept interrumpido. Cerrando...\n");
        } else {
            perror("[Error] accept");
        }
        close(listenfd);
        return EXIT_FAILURE;
    }

    // Convertir dirección IP a string legible
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliaddr.sin_addr, ipstr, sizeof(ipstr));
    
    printf("✓ Cliente conectado desde %s:%d\n", ipstr, ntohs(cliaddr.sin_port));
    printf("\n──────────────────────────────────────────────────────────\n");
    printf("  Comandos disponibles:\n");
    printf("    /quit  - Cerrar la conexión\n");
    printf("    Ctrl+C - Terminar el servidor\n");
    printf("    Ctrl+D - Cerrar conexión (EOF)\n");
    printf("──────────────────────────────────────────────────────────\n\n");

    // Ya no necesitamos el socket de escucha
    close(listenfd);

    // ========================================================================
    // 7. CREACIÓN DE THREADS PARA I/O CONCURRENTE
    // ========================================================================
    
    thread_args_t args = { 
        .sockfd = connfd,
        .mutex = &running_mutex 
    };
    
    pthread_t th_recv, th_send;

    // Thread para recibir mensajes del cliente
    if (pthread_create(&th_recv, NULL, recv_thread, &args) != 0) {
        perror("[Error] pthread_create recv");
        close(connfd);
        return EXIT_FAILURE;
    }

    // Thread para enviar mensajes al cliente
    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("[Error] pthread_create send");
        set_running(0);
        pthread_join(th_recv, NULL);  // Esperar al thread de recepción
        close(connfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 8. ESPERAR FINALIZACIÓN DE THREADS
    // ========================================================================
    
    // pthread_join() espera a que el thread termine
    // Es importante esperar ambos threads antes de cerrar recursos
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    // ========================================================================
    // 9. LIMPIEZA Y CIERRE
    // ========================================================================
    
    close(connfd);
    
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Servidor cerrado correctamente\n");
    printf("═══════════════════════════════════════════════════════════\n");

    // Destruir mutex
    pthread_mutex_destroy(&running_mutex);

    return EXIT_SUCCESS;
}