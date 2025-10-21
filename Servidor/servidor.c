// ============================================================================
// servidor.c - Chat 1:1 con sockets + threads
// ============================================================================
// Compilar: gcc servidor.c -o servidor -pthread
// Ejecutar: ./servidor 5000
//
// Este programa crea un servidor que:
// 1. Espera que un cliente se conecte
// 2. Permite chatear bidireccionalmente (ambos envían/reciben a la vez)
// 3. Usa 2 threads para poder enviar y recibir simultáneamente
// ============================================================================

#include <stdio.h>      // printf, perror, fprintf
#include <stdlib.h>     // exit, atoi, EXIT_FAILURE
#include <string.h>     // strlen, strncmp
#include <signal.h>     // signal, SIGINT
#include <errno.h>      // errno, EINTR
#include <unistd.h>     // close
#include <arpa/inet.h>  // socket, bind, listen, accept, htons, inet_ntop
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

// Este thread se encarga SOLO de recibir mensajes del cliente
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
            printf("\n[Servidor] Cliente desconectado.\n");
            running = 0;  // Señal para que el otro thread termine
            break;
        }

        // Si llegamos acá, recibimos datos correctamente
        buf[n] = '\0';  // Agregar terminador de string (para usar printf)
        printf("\nCliente: %s", buf);
        fflush(stdout);  // Forzar que se imprima inmediatamente
    }

    return NULL;  // Los threads deben retornar algo
}

// ============================================================================
// THREAD DE ENVÍO
// ============================================================================

// Este thread se encarga SOLO de enviar mensajes al cliente
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
    // Verificar que se pasó el puerto como argumento
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Configurar handler para Ctrl+C (señal SIGINT)
    signal(SIGINT, handle_sigint);
    
    // Convertir string a número
    int port = atoi(argv[1]);
    
    // ========================================================================
    // PASO 1: Crear socket
    // ========================================================================
    // socket() crea un nuevo socket y retorna su file descriptor
    // Parámetros:
    //   AF_INET = familia IPv4
    //   SOCK_STREAM = TCP (orientado a conexión, confiable)
    //   0 = protocolo por defecto (TCP para SOCK_STREAM)
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Opción SO_REUSEADDR permite reutilizar el puerto inmediatamente
    // Sin esto, después de cerrar el servidor hay que esperar ~1 minuto
    // para volver a usar el mismo puerto (estado TIME_WAIT)
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // ========================================================================
    // PASO 2: Configurar dirección
    // ========================================================================
    // struct sockaddr_in guarda: familia, IP y puerto
    struct sockaddr_in addr = {0};  // {0} inicializa todo en ceros
    addr.sin_family = AF_INET;      // IPv4
    
    // INADDR_ANY = 0.0.0.0 = escuchar en TODAS las interfaces de red
    // (localhost, ethernet, wifi, etc.)
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // htons = Host TO Network Short
    // Convierte el puerto del formato de la máquina al formato de red
    // (big-endian). Necesario para compatibilidad entre arquitecturas.
    addr.sin_port = htons((uint16_t)port);

    // ========================================================================
    // PASO 3: Bind (asociar socket con dirección y puerto)
    // ========================================================================
    // bind() asocia el socket con una dirección IP y puerto específicos
    // Después de esto, nadie más puede usar ese puerto
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // PASO 4: Listen (marcar socket como pasivo)
    // ========================================================================
    // listen() marca el socket como "servidor" (pasivo)
    // Ahora puede recibir conexiones entrantes
    // El parámetro 1 es el "backlog": cuántas conexiones pueden estar
    // esperando en cola mientras procesamos otra
    if (listen(listenfd, 1) < 0) {
        perror("listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    printf("[Servidor] Esperando conexión en puerto %d...\n", port);

    // ========================================================================
    // PASO 5: Accept (aceptar conexión entrante)
    // ========================================================================
    // accept() es BLOQUEANTE: se queda esperando hasta que llegue un cliente
    // Cuando llega un cliente, crea un NUEVO socket para esa conexión
    // y retorna su file descriptor
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    
    if (connfd < 0) {
        perror("accept");
        close(listenfd);
        return EXIT_FAILURE;
    }

    // inet_ntop = Internet Network TO Presentation
    // Convierte la IP del cliente de binario a string legible
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliaddr.sin_addr, ipstr, sizeof(ipstr));
    
    // ntohs = Network TO Host Short
    // Convierte el puerto de formato de red a formato de la máquina
    printf("[Servidor] Cliente conectado: %s:%d\n", ipstr, ntohs(cliaddr.sin_port));
    printf("Escribí mensajes y presioná Enter. Comando: /quit\n\n");

    // Ya no necesitamos el socket de escucha (listenfd)
    // Solo usaremos connfd para comunicarnos con este cliente
    close(listenfd);

    // ========================================================================
    // PASO 6: Crear threads para I/O concurrente
    // ========================================================================
    // Preparar argumentos para los threads
    thread_args_t args = { .sockfd = connfd };
    
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
        close(connfd);
        return EXIT_FAILURE;
    }

    if (pthread_create(&th_send, NULL, send_thread, &args) != 0) {
        perror("pthread_create send");
        running = 0;
        pthread_join(th_recv, NULL);  // Limpiar el thread que sí se creó
        close(connfd);
        return EXIT_FAILURE;
    }

    // ========================================================================
    // PASO 7: Esperar a que terminen los threads
    // ========================================================================
    // pthread_join() espera a que un thread termine
    // Es como wait() para procesos, pero para threads
    // IMPORTANTE: siempre hay que hacer join de los threads que creamos
    // sino quedan como "zombies"
    pthread_join(th_send, NULL);
    pthread_join(th_recv, NULL);

    // ========================================================================
    // PASO 8: Limpieza
    // ========================================================================
    close(connfd);
    printf("\n[Servidor] Cerrado.\n");

    return EXIT_SUCCESS;
}