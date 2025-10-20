// ============================================================================
// cliente.c - Chat 1:1 con sockets + threads (lado cliente)
// ============================================================================
// Ejemplo educativo de programación con sockets TCP y threads en C
// 
// Compilar: gcc cliente.c -o cliente -pthread
// Ejecutar: ./cliente 127.0.0.1 5000
// 
// Este programa demuestra:
//   - Conexión a servidor TCP (socket, connect)
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
#define MIN_PORT 1024          // Puerto mínimo permitido
#define MAX_PORT 65535         // Puerto máximo permitido

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
    printf("\n[Cliente] Señal de interrupción recibida. Cerrando...\n");
    set_running(0);
}


// ============================================================================
// THREAD DE RECEPCIÓN
// ============================================================================

/**
 * Thread que recibe mensajes del servidor
 * Se ejecuta concurrentemente con el thread de envío
 * 
 * @param arg: puntero a thread_args_t con socket y mutex
 * @return NULL al terminar
 */
static void* recv_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    char buf[BUF_SIZE];

    while (is_running()) {
        // recv() bloqueante: espera datos del servidor
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
                perror("[Cliente] Error en recv");
            }
            break;
        } else if (n == 0) {
            // n == 0 significa que el peer cerró la conexión ordenadamente
            printf("\n[Cliente] El servidor cerró la conexión.\n");
            set_running(0);
            break;
        }

        // Agregar terminador nulo y mostrar mensaje
        buf[n] = '\0';
        
        // Imprimir con formato para mejor visualización
        printf("\n┌─[Servidor]───────────────────────────────────────────────┐\n");
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
 * Thread que envía mensajes al servidor
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
            printf("\n[Cliente] EOF detectado en stdin.\n");
            shutdown(args->sockfd, SHUT_WR);
            set_running(0);
            break;
        }

        // Comando especial para salir
        if (strncmp(line, "/quit", 5) == 0) {
            printf("[Cliente] Comando /quit recibido. Cerrando...\n");
            shutdown(args->sockfd, SHUT_WR);
            set_running(0);
            break;
        }

        // Enviar el mensaje al servidor
        size_t len = strlen(line);
        ssize_t sent = send(args->sockfd, line, len, 0);
        
        if (sent < 0) {
            perror("[Cliente] Error en send");
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

/**
 * Valida que una cadena sea una dirección IPv4 válida
 * 
 * @param ip: cadena con dirección IP a validar
 * @return 1 si es válida, 0 si no
 */
static int validate_ip(const char* ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}

int main(int argc, char* argv[]) {
    // ========================================================================
    // 1. VALIDACIÓN DE ARGUMENTOS
    // ========================================================================
    
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        fprintf(stderr, "  ip_servidor: dirección IPv4 del servidor (ej: 127.0.0.1)\n");
        fprintf(stderr, "  puerto: número entre %d y %d\n", MIN_PORT, MAX_PORT);
        fprintf(stderr, "Ejemplo: %s 127.0.0.1 5000\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // Validar dirección IP
    if (!validate_ip(ip)) {
        fprintf(stderr, "[Error] Dirección IP inválida: %s\n", ip);
        fprintf(stderr, "Proporciona una dirección IPv4 válida (ej: 192.168.1.100)\n");
        return EXIT_FAILURE;
    }

    // Validar puerto
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
    signal(SIGPIPE, SIG_IGN);

    // ========================================================================
    // 3. CREACIÓN DEL SOCKET
    // ========================================================================
    
    // socket() crea un endpoint de comunicación
    // AF_INET: IPv4, SOCK_STREAM: TCP, 0: protocolo por defecto
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[Error] socket");
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 4. CONFIGURACIÓN DE LA DIRECCIÓN DEL SERVIDOR
    // ========================================================================
    
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)port);  // htons: host to network short

    // inet_pton: convierte string IP a formato binario
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("[Error] inet_pton");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 5. CONECTAR AL SERVIDOR
    // ========================================================================
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  CLIENTE DE CHAT\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Conectando a %s:%d...\n", ip, port);
    
    // connect() es bloqueante hasta que se establezca la conexión
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("[Error] connect");
        fprintf(stderr, "\n¿El servidor está ejecutándose en %s:%d?\n", ip, port);
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("  ✓ Conectado exitosamente\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n──────────────────────────────────────────────────────────\n");
    printf("  Comandos disponibles:\n");
    printf("    /quit  - Cerrar la conexión\n");
    printf("    Ctrl+C - Terminar el cliente\n");
    printf("    Ctrl+D - Cerrar conexión (EOF)\n");
    printf("──────────────────────────────────────────────────────────\n\n");

    // ========================================================================
    // 6. CREACIÓN DE THREADS PARA I/O CONCURRENTE
    // ========================================================================
    
    thread_args_t args = { 
        .sockfd = sockfd,
        .mutex = &running_mutex 
    };
    
    pthread_t th_recv, th_send;

    // Thread para recibir mensajes del servidor
    if (pthread_create(&th_recv, NULL, recv_thread, &args) != 0) {
        perror("[Error] pthread_create recv");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Thread para enviar mensajes al servidor
    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("[Error] pthread_create send");
        set_running(0);
        pthread_join(th_recv, NULL);  // Esperar al thread de recepción
        close(sockfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // 7. ESPERAR FINALIZACIÓN DE THREADS
    // ========================================================================
    
    // pthread_join() espera a que el thread termine
    // Es importante esperar ambos threads antes de cerrar recursos
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    // ========================================================================
    // 8. LIMPIEZA Y CIERRE
    // ========================================================================
    
    close(sockfd);
    
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Cliente cerrado correctamente\n");
    printf("═══════════════════════════════════════════════════════════\n");

    // Destruir mutex
    pthread_mutex_destroy(&running_mutex);

    return EXIT_SUCCESS;
}