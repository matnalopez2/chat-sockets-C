git status
# 📖 Guía Rápida de Referencia

> Referencia rápida de funciones, conceptos y comandos para el proyecto de Chat con Sockets

---

## 🔌 Funciones de Sockets (API POSIX)

### `socket()` - Crear un socket
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```
- **AF_INET**: Familia IPv4
- **SOCK_STREAM**: TCP (orientado a conexión)
- **Retorna**: File descriptor del socket, o -1 si error

---

### `bind()` - Asociar socket con dirección
```c
int result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
```
- Asocia un socket con una dirección IP y puerto
- Solo lo usa el **servidor**
- **Retorna**: 0 si éxito, -1 si error

---

### `listen()` - Marcar socket como pasivo
```c
int result = listen(sockfd, backlog);
```
- Marca el socket para aceptar conexiones entrantes
- **backlog**: Número máximo de conexiones pendientes en cola
- Solo lo usa el **servidor**
- **Retorna**: 0 si éxito, -1 si error

---

### `accept()` - Aceptar conexión entrante
```c
int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
```
- **Bloqueante**: Espera hasta que llegue una conexión
- Retorna un **nuevo socket** para comunicarse con el cliente
- Solo lo usa el **servidor**
- **Retorna**: File descriptor del socket conectado, o -1 si error

---

### `connect()` - Conectar a un servidor
```c
int result = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
```
- Establece conexión con el servidor
- Solo lo usa el **cliente**
- **Retorna**: 0 si éxito, -1 si error

---

### `send()` - Enviar datos
```c
ssize_t sent = send(sockfd, buffer, length, flags);
```
- Envía datos por el socket
- **buffer**: Puntero a los datos a enviar
- **length**: Número de bytes a enviar
- **flags**: Usualmente 0
- **Retorna**: Número de bytes enviados, o -1 si error

---

### `recv()` - Recibir datos
```c
ssize_t n = recv(sockfd, buffer, size, flags);
```
- **Bloqueante**: Espera hasta recibir datos
- **Retorna**:
  - `> 0`: Número de bytes recibidos
  - `0`: El peer cerró la conexión
  - `-1`: Error

---

### `shutdown()` - Cierre parcial
```c
shutdown(sockfd, SHUT_WR);   // Cierra escritura
shutdown(sockfd, SHUT_RD);   // Cierra lectura
shutdown(sockfd, SHUT_RDWR); // Cierra ambos
```
- Permite cierre "half-duplex"
- **SHUT_WR**: El peer recibe EOF, pero aún podemos leer

---

### `close()` - Cerrar socket
```c
close(sockfd);
```
- Cierra completamente el socket
- Libera el file descriptor

---

## 🧵 Funciones de Threads (POSIX Threads)

### `pthread_create()` - Crear un thread
```c
pthread_t thread_id;
int result = pthread_create(&thread_id, NULL, thread_function, arg);
```
- **thread_id**: Variable donde se guarda el ID del thread
- **NULL**: Atributos por defecto
- **thread_function**: Función que ejecutará el thread (tipo `void* (*)(void*)`)
- **arg**: Argumento pasado a la función
- **Retorna**: 0 si éxito, número de error si falla

---

### `pthread_join()` - Esperar a que termine un thread
```c
pthread_join(thread_id, NULL);
```
- **Bloqueante**: Espera hasta que el thread termine
- Similar a `wait()` para procesos
- Necesario para evitar threads "zombie"

---

### `pthread_mutex_lock()` - Bloquear mutex
```c
pthread_mutex_lock(&mutex);
// Sección crítica protegida
pthread_mutex_unlock(&mutex);
```
- Asegura que solo un thread acceda a la sección crítica a la vez
- Si el mutex está bloqueado, espera hasta que se libere

---

### `pthread_mutex_unlock()` - Desbloquear mutex
```c
pthread_mutex_unlock(&mutex);
```
- Libera el mutex para que otro thread pueda usarlo
- Debe ser llamado por el mismo thread que hizo el lock

---

### `pthread_mutex_destroy()` - Destruir mutex
```c
pthread_mutex_destroy(&mutex);
```
- Libera recursos asociados al mutex
- Llamar antes de terminar el programa

---

## 🌐 Conversión de Direcciones

### `inet_pton()` - String a binario
```c
inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr);
```
- Convierte IP en formato texto a formato binario
- **p**resentation **to** **n**etwork
- **Retorna**: 1 si éxito, 0 si formato inválido, -1 si error

---

### `inet_ntop()` - Binario a string
```c
char ip_str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
```
- Convierte IP en formato binario a formato texto
- **n**etwork **to** **p**resentation
- **Retorna**: Puntero al string, o NULL si error

---

### `htons()` / `htonl()` - Host to Network
```c
uint16_t net_port = htons(5000);    // short (16 bits)
uint32_t net_addr = htonl(addr);    // long (32 bits)
```
- Convierte del byte order del host al byte order de red
- Red usa **big-endian**, algunos CPUs usan little-endian

---

### `ntohs()` / `ntohl()` - Network to Host
```c
uint16_t host_port = ntohs(net_port);
uint32_t host_addr = ntohl(net_addr);
```
- Convierte del byte order de red al byte order del host

---

## 📡 Estructura `sockaddr_in`

```c
struct sockaddr_in {
    sa_family_t    sin_family;   // AF_INET para IPv4
    in_port_t      sin_port;     // Puerto (en network byte order)
    struct in_addr sin_addr;     // Dirección IP
    char           sin_zero[8];  // Padding (ignorado)
};
```

### Ejemplo de uso:
```c
struct sockaddr_in addr = {0};
addr.sin_family = AF_INET;
addr.sin_port = htons(5000);
addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 0.0.0.0 (todas las interfaces)
```

### Direcciones especiales:
- **INADDR_ANY** (`0.0.0.0`): Escucha en todas las interfaces
- **INADDR_LOOPBACK** (`127.0.0.1`): Localhost

---

## 🚦 Manejo de Señales

### `signal()` - Registrar manejador
```c
signal(SIGINT, handle_sigint);  // Capturar Ctrl+C
signal(SIGPIPE, SIG_IGN);       // Ignorar SIGPIPE
```

### Señales comunes:
- **SIGINT**: Ctrl+C (interrupción)
- **SIGTERM**: Solicitud de terminación
- **SIGPIPE**: Escritura en socket cerrado
- **SIGUSR1/SIGUSR2**: Señales definidas por usuario

### Función manejadora:
```c
void handle_sigint(int sig) {
    // No llamar funciones no async-signal-safe aquí
    // Solo modificar variables volatile sig_atomic_t
    printf("Señal %d recibida\n", sig);
}
```

---

## 🔍 Manejo de Errores

### `perror()` - Imprimir error del sistema
```c
if (result < 0) {
    perror("socket");  // Imprime: "socket: <descripción del error>"
}
```

### `errno` - Variable global con código de error
```c
#include <errno.h>

if (recv_result < 0) {
    if (errno == EINTR) {
        // Interrumpido por señal, reintentar
    } else if (errno == EAGAIN) {
        // No hay datos disponibles (socket no bloqueante)
    }
}
```

### Códigos de error comunes:
- **EINTR**: Interrumpido por señal
- **EAGAIN/EWOULDBLOCK**: Operación bloqueante, reintentar
- **ECONNRESET**: Conexión reseteada por el peer
- **EADDRINUSE**: Dirección ya en uso
- **ETIMEDOUT**: Timeout de conexión

---

## 🛠️ Comandos Útiles

### Compilación:
```bash
# Con warnings habilitados
gcc -Wall -Wextra -std=c11 -pthread programa.c -o programa

# Con símbolos de depuración
gcc -g -Wall -pthread programa.c -o programa

# Con optimización
gcc -O2 -Wall -pthread programa.c -o programa
```

---

### Depuración:
```bash
# Con gdb
gdb ./programa
(gdb) break main
(gdb) run arg1 arg2
(gdb) next
(gdb) print variable
(gdb) bt  # backtrace

# Con valgrind (detectar memory leaks)
valgrind --leak-check=full ./programa
```

---

### Análisis de red:
```bash
# Ver puertos en escucha
netstat -tulnp | grep 5000
# o con ss (más moderno)
ss -tulnp | grep 5000

# Ver conexiones activas
netstat -an | grep ESTABLISHED

# Capturar tráfico
tcpdump -i lo port 5000 -X

# Verificar si un puerto está abierto
nc -zv 127.0.0.1 5000

# Conectar manualmente (como cliente)
nc 127.0.0.1 5000
```

---

### Monitoreo de procesos:
```bash
# Ver threads de un proceso
ps -T -p <PID>

# Ver uso de recursos
top -H -p <PID>

# Ver archivos abiertos (incluye sockets)
lsof -p <PID>

# Ver sockets de un proceso
lsof -i -a -p <PID>
```

---

## 📊 Diferencias Clave

### TCP vs UDP

| Característica | TCP | UDP |
|---------------|-----|-----|
| **Tipo** | SOCK_STREAM | SOCK_DGRAM |
| **Conexión** | Orientado a conexión | Sin conexión |
| **Fiabilidad** | Garantiza entrega ordenada | No garantiza nada |
| **Velocidad** | Más lento | Más rápido |
| **Uso** | Web, email, FTP, SSH | Streaming, juegos, DNS |

---

### Bloqueante vs No Bloqueante

| Modo | Comportamiento |
|------|----------------|
| **Bloqueante** | `recv()` espera hasta recibir datos |
| **No bloqueante** | `recv()` retorna inmediatamente (errno = EAGAIN) |

```c
// Hacer socket no bloqueante
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
```

---

### `shutdown()` vs `close()`

| Función | Efecto | Cuándo usar |
|---------|--------|-------------|
| `shutdown(fd, SHUT_WR)` | Cierra solo escritura | Señalar "ya no enviaré más" pero seguir recibiendo |
| `close(fd)` | Cierra todo | Terminar comunicación completamente |

---

## 🎯 Checklist de Buenas Prácticas

### ✅ Validación de Entrada
- [ ] Validar argumentos de línea de comandos
- [ ] Verificar rango de puertos (1024-65535)
- [ ] Validar formato de direcciones IP

### ✅ Manejo de Errores
- [ ] Verificar retorno de todas las funciones del sistema
- [ ] Usar `perror()` o `strerror()` para mensajes descriptivos
- [ ] Manejar `EINTR` correctamente (reintentar)

### ✅ Gestión de Recursos
- [ ] Cerrar todos los sockets con `close()`
- [ ] Hacer `pthread_join()` de todos los threads creados
- [ ] Destruir todos los mutex con `pthread_mutex_destroy()`
- [ ] No fugas de memoria (verificar con valgrind)

### ✅ Concurrencia
- [ ] Proteger variables compartidas con mutex
- [ ] Evitar deadlocks (siempre bloquear en el mismo orden)
- [ ] No llamar funciones no thread-safe sin protección

### ✅ Señales
- [ ] Capturar `SIGINT` para cierre ordenado
- [ ] Ignorar `SIGPIPE` (manejar errores en `send()`)
- [ ] Solo usar funciones async-signal-safe en handlers

### ✅ Compilación
- [ ] Compilar con `-Wall -Wextra` y corregir todos los warnings
- [ ] Probar con `-Werror` (warnings como errores)
- [ ] Usar `-std=c11` o superior

---

## 🔗 Man Pages Esenciales

```bash
# Sockets
man 2 socket
man 2 bind
man 2 listen
man 2 accept
man 2 connect
man 2 send
man 2 recv
man 2 shutdown
man 2 close

# Threads
man 3 pthread_create
man 3 pthread_join
man 3 pthread_mutex_lock
man 3 pthread_mutex_unlock

# Direcciones
man 3 inet_pton
man 3 inet_ntop
man 3 htons

# Estructuras
man 7 ip
man 7 tcp
man 2 socket (para struct sockaddr)
```

---

## 💡 Tips y Trucos

### 1. Puerto ocupado (EADDRINUSE)
```c
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

### 2. Evitar buffers parciales
```c
// Enviar todos los bytes
ssize_t total_sent = 0;
while (total_sent < length) {
    ssize_t sent = send(sockfd, buffer + total_sent, length - total_sent, 0);
    if (sent < 0) {
        perror("send");
        break;
    }
    total_sent += sent;
}
```

### 3. Timeout en recv
```c
struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
```

### 4. Ver tamaño de buffer
```c
int bufsize;
socklen_t len = sizeof(bufsize);
getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, &len);
printf("Buffer de recepción: %d bytes\n", bufsize);
```

---

## 🐛 Errores Comunes

### ❌ Olvidar `htons()`
```c
// MAL
addr.sin_port = 5000;

// BIEN
addr.sin_port = htons(5000);
```

### ❌ No verificar retornos
```c
// MAL
recv(sockfd, buf, sizeof(buf), 0);

// BIEN
ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
if (n < 0) { perror("recv"); }
```

### ❌ Buffer sin terminador nulo
```c
// MAL
recv(sockfd, buf, sizeof(buf), 0);
printf("%s\n", buf);  // ⚠️ Puede imprimir basura

// BIEN
ssize_t n = recv(sockfd, buf, sizeof(buf) - 1, 0);
buf[n] = '\0';
printf("%s\n", buf);
```

### ❌ Race condition sin mutex
```c
// MAL
if (running) {  // Thread 1 lee
    running = 0;  // Thread 2 escribe ⚠️
}

// BIEN
pthread_mutex_lock(&mutex);
if (running) {
    running = 0;
}
pthread_mutex_unlock(&mutex);
```

---

**¡Guarda esta guía como referencia durante el desarrollo! 📌**

