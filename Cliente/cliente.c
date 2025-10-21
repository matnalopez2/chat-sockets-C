// ============================================================================
// cliente.c - Chat 1:1 con sockets + threads
// ============================================================================
// Compilar: gcc cliente.c -o cliente -pthread
// Ejecutar: ./cliente 127.0.0.1 5000
//
// Este programa crea un cliente que:
// 1. Se conecta a un servidor
// 2. Permite chatear bidireccionalmente (ambos envían/reciben a la vez)
// 3. Usa 2 threads para poder enviar y recibir simultáneamente
// ============================================================================

#include <stdio.h>      // printf, perror, fprintf
#include <stdlib.h>     // exit, atoi, EXIT_FAILURE
#include <string.h>     // strlen, strncmp
#include <signal.h>     // signal, SIGINT
#include <errno.h>      // errno, EINTR
#include <unistd.h>     // close
#include <arpa/inet.h>  // socket, connect, inet_pton
#include <pthread.h>    // pthread_create, pthread_join

#define BUF_SIZE 1024   // Tamaño del buffer para mensajes

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

// Variable que indica si el programa debe seguir corriendo
// - volatile: le dice al compilador que no optimice accesos a esta variable
//   porque puede cambiar desde afuera (por ejemplo, desde un signal handler)
// - sig_atomic_t: tipo especial que garantiza lectura/escritura atómica
//   (no se puede interrumpir a la mitad). Necesario para signal handlers.
static volatile sig_atomic_t running = 1;

// ============================================================================
// ESTRUCTURAS
// ============================================================================

// Estructura para pasar argumentos a los threads
// Cuando creamos un thread, podemos pasarle un solo puntero void*
// Usamos esta estructura para pasar múltiples datos
typedef struct {
    int sockfd;  // File descriptor del socket para comunicación
} thread_args_t;

// ============================================================================
// SIGNAL HANDLERS
// ============================================================================

// Función que se ejecuta cuando el usuario presiona Ctrl+C
// Al poner running = 0, los loops de los threads terminan
static void handle_sigint(int sig) {
    (void)sig;  // Evitar warning de "parámetro no usado"
    running = 0;
}

// ============================================================================
// THREAD DE RECEPCIÓN
// ============================================================================

// Este thread se encarga SOLO de recibir mensajes del servidor
// Corre en paralelo con send_thread, por eso podemos recibir y enviar a la vez
//
// Parámetros:
//   arg: puntero a thread_args_t con el socket
// Retorna:
//   NULL cuando termina
static void* recv_thread(void* arg) {
    // Convertir el puntero void* al tipo correcto
    thread_args_t* args = (thread_args_t*)arg;
    
    // Buffer para guardar los mensajes recibidos
    char buf[BUF_SIZE];

    // Loop principal: mientras el programa esté corriendo
    while (running) {
        // recv() es una función BLOQUEANTE:
        // - Se queda esperando hasta que lleguen datos
        // - Retorna la cantidad de bytes recibidos
        // - Dejamos 1 byte libre (sizeof - 1) para el '\0' al final
        ssize_t n = recv(args->sockfd, buf, sizeof(buf) - 1, 0);
        
        // Revisar errores
        if (n < 0) {
            // errno es una variable global que contiene el código de error
            if (errno == EINTR) {
                // EINTR = la función fue interrumpida por una señal (ej: Ctrl+C)
                // No es un error real, solo reintentamos
                continue;
            }
            // Si es otro error, mostrar mensaje y salir
            perror("recv");
            break;
        } else if (n == 0) {
            // recv() retorna 0 cuando el otro lado cerró la conexión
            // Es la forma correcta de detectar desconexión
            printf("\n[Cliente] Servidor desconectado.\n");
            running = 0;  // Señal para que el otro thread termine
            break;
        }

        // Si llegamos acá, recibimos datos correctamente
        buf[n] = '\0';  // Agregar terminador de string (para usar printf)
        printf("\nServidor: %s", buf);
        fflush(stdout);  // Forzar que se imprima inmediatamente
    }

    return NULL;  // Los threads deben retornar algo
}

// ============================================================================
// THREAD DE ENVÍO
// ============================================================================

// Este thread se encarga SOLO de enviar mensajes al servidor
// Lee del teclado y envía por el socket
//
// Parámetros:
//   arg: puntero a thread_args_t con el socket
// Retorna:
//   NULL cuando termina
static void* send_thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    
    // Buffer para la línea que escribe el usuario
    char line[BUF_SIZE];

    // Loop principal: mientras el programa esté corriendo
    while (running) {
        // fgets() es una función BLOQUEANTE:
        // - Se queda esperando hasta que el usuario escriba algo y presione Enter
        // - Lee hasta encontrar '\n' o hasta llenar el buffer
        // - Retorna NULL si hay EOF (Ctrl+D) o error
        if (!fgets(line, sizeof(line), stdin)) {
            // EOF en stdin (usuario presionó Ctrl+D)
            // shutdown() cierra solo el envío, pero podemos seguir recibiendo
            shutdown(args->sockfd, SHUT_WR);
            break;
        }
        
        // Comando especial para salir del chat
        if (strncmp(line, "/quit", 5) == 0) {
            shutdown(args->sockfd, SHUT_WR);
            running = 0;  // Señal para que el otro thread termine
            break;
        }

        // send() envía datos por el socket
        // - Primer parámetro: socket
        // - Segundo: buffer con los datos
        // - Tercero: cantidad de bytes a enviar
        // - Cuarto: flags (0 = comportamiento normal)
        if (send(args->sockfd, line, strlen(line), 0) < 0) {
            perror("send");
            running = 0;
            break;
        }
    }
    
    return NULL;
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char* argv[]) {
    // Verificar que se pasaron IP y puerto como argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Configurar handler para Ctrl+C (señal SIGINT)
    signal(SIGINT, handle_sigint);

    const char* ip = argv[1];
    int port = atoi(argv[2]);  // Convertir string a número

    // ========================================================================
    // PASO 1: Crear socket
    // ========================================================================
    // socket() crea un nuevo socket y retorna su file descriptor
    // Parámetros:
    //   AF_INET = familia IPv4
    //   SOCK_STREAM = TCP (orientado a conexión, confiable)
    //   0 = protocolo por defecto (TCP para SOCK_STREAM)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // ========================================================================
    // PASO 2: Configurar dirección del servidor
    // ========================================================================
    // struct sockaddr_in guarda: familia, IP y puerto
    struct sockaddr_in servaddr = {0};  // {0} inicializa todo en ceros
    servaddr.sin_family = AF_INET;      // IPv4
    
    // htons = Host TO Network Short
    // Convierte el puerto del formato de la máquina al formato de red
    // (big-endian). Necesario para compatibilidad entre arquitecturas.
    servaddr.sin_port = htons((uint16_t)port);

    // inet_pton = Internet Presentation TO Network
    // Convierte la IP de string (ej: "192.168.1.1") a formato binario
    // Parámetros:
    //   AF_INET = IPv4
    //   ip = string con la IP
    //   &servaddr.sin_addr = donde guardar el resultado
    // Retorna: 1 si OK, 0 si formato inválido, -1 si error
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // PASO 3: Connect (conectar al servidor)
    // ========================================================================
    // connect() intenta conectarse al servidor
    // Es BLOQUEANTE: espera hasta que se establezca la conexión o falle
    // Inicia el "three-way handshake" de TCP (SYN, SYN-ACK, ACK)
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("[Cliente] Conectado a %s:%d\n", ip, port);
    printf("Escribí mensajes y presioná Enter. Comando: /quit\n\n");

    // ========================================================================
    // PASO 4: Crear threads para I/O concurrente
    // ========================================================================
    // Preparar argumentos para los threads
    thread_args_t args = { .sockfd = sockfd };
    
    // Variables para guardar los IDs de los threads
    pthread_t th_recv, th_send;

    // pthread_create() crea y ejecuta un nuevo thread
    // Parámetros:
    //   1. Puntero donde guardar el ID del thread
    //   2. Atributos (NULL = por defecto)
    //   3. Función que ejecutará el thread
    //   4. Argumento para pasarle a esa función
    if (pthread_create(&th_recv, NULL, recv_thread, &args) != 0) {
        perror("pthread_create recv");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("pthread_create send");
        running = 0;
        pthread_join(th_recv, NULL);  // Limpiar el thread que sí se creó
        close(sockfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // PASO 5: Esperar a que terminen los threads
    // ========================================================================
    // pthread_join() espera a que un thread termine
    // Es como wait() para procesos, pero para threads
    // IMPORTANTE: siempre hay que hacer join de los threads que creamos
    // sino quedan como "zombies"
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    // ========================================================================
    // PASO 6: Limpieza
    // ========================================================================
    close(sockfd);
    printf("\n[Cliente] Cerrado.\n");
    
    return EXIT_SUCCESS;
}
