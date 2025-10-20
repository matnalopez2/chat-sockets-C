# Chat con Sockets y Threads en C

Ejemplo simple de chat cliente-servidor usando sockets TCP y threads POSIX.

## ¿Qué hace?

- **Servidor**: Espera conexiones y chatéa con un cliente
- **Cliente**: Se conecta al servidor y chatéa

Ambos pueden enviar y recibir mensajes **al mismo tiempo** gracias a los threads.

## Compilar

```bash
# Servidor
gcc Servidor/servidor.c -o Servidor/servidor -pthread

# Cliente
gcc Cliente/cliente.c -o Cliente/cliente -pthread
```

O simplemente:
```bash
make
```

## Usar

**Terminal 1 - Servidor:**
```bash
cd Servidor
./servidor 5000
```

**Terminal 2 - Cliente:**
```bash
cd Cliente
./cliente 127.0.0.1 5000
```

Ahora escribí mensajes en cualquiera de las dos terminales y presioná Enter.

Para salir: escribí `/quit` o presioná Ctrl+C

## ¿Cómo funciona?

### Conceptos Clave

1. **Sockets**: Permiten comunicación entre programas por red
   - `socket()` - Crea el socket
   - `bind()` - Asocia socket con puerto (servidor)
   - `listen()` - Espera conexiones (servidor)
   - `accept()` - Acepta cliente (servidor)
   - `connect()` - Conecta a servidor (cliente)
   - `send()` / `recv()` - Envía/recibe datos

2. **Threads**: Permiten hacer 2 cosas a la vez
   - Thread 1: Recibe mensajes (con `recv()`)
   - Thread 2: Envía mensajes (con `send()`)
   - Si no usáramos threads, no podríamos escribir mientras esperamos mensajes

3. **TCP**: Protocolo que garantiza que los datos llegan en orden y sin errores

### Estructura del Servidor

```
1. Crear socket TCP
2. Configurar dirección (IP + puerto)
3. Bind: asociar socket con puerto
4. Listen: marcar socket como "servidor"
5. Accept: esperar cliente (se queda esperando aquí)
6. Crear 2 threads:
   - Thread recepción: loop de recv()
   - Thread envío: loop de fgets() + send()
7. Esperar a que terminen los threads
8. Cerrar todo
```

### Estructura del Cliente

```
1. Crear socket TCP
2. Configurar dirección del servidor
3. Connect: conectar al servidor
4. Crear 2 threads:
   - Thread recepción: loop de recv()
   - Thread envío: loop de fgets() + send()
5. Esperar a que terminen los threads
6. Cerrar todo
```

## Diagrama

```
SERVIDOR                 CLIENTE
   |                        |
socket()                 socket()
   |                        |
bind()                     |
   |                        |
listen()                   |
   |                        |
accept() <--conecta---  connect()
   |                        |
   |<---datos bidi--->      |
   |                        |
```

## Variables Importantes

```c
volatile sig_atomic_t running = 1;
```
- **volatile**: Le dice al compilador que no optimice esta variable porque puede cambiar "mágicamente" (desde otro thread o signal handler)
- **sig_atomic_t**: Tipo especial para usar en signal handlers de forma segura
- Cuando vale 1, los threads siguen ejecutándose. Cuando vale 0, terminan.

```c
typedef struct {
    int sockfd;  // File descriptor del socket
} thread_args_t;
```
Estructura para pasar datos a los threads.

## Funciones de Red

```c
// Crear socket TCP
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//                   IPv4     TCP

// Enviar datos
send(sockfd, buffer, tamaño, 0);

// Recibir datos (bloqueante: espera hasta que lleguen)
ssize_t n = recv(sockfd, buffer, tamaño, 0);
// Retorna: cantidad de bytes recibidos, o 0 si se cerró, o -1 si error
```

## Funciones de Threads

```c
pthread_t thread_id;

// Crear thread
pthread_create(&thread_id, NULL, mi_funcion, argumentos);

// Esperar a que termine
pthread_join(thread_id, NULL);
```

## ¿Por qué 2 threads?

Porque `recv()` y `fgets()` son **bloqueantes**:
- `recv()` se queda esperando hasta que lleguen datos
- `fgets()` se queda esperando hasta que escribas algo

Si todo estuviera en un solo thread:
```
recv() → espera mensaje...
       (no puedo escribir mientras espero!)
```

Con 2 threads:
```
Thread 1: recv() → espera mensajes
Thread 2: fgets() → espera que escribas

¡Los dos corren en paralelo!
```

## Ejercicios

1. **Básico**: Agregá un timestamp a cada mensaje (usar `time()`)

2. **Intermedio**: Modificá el servidor para que soporte 2 clientes simultáneos

3. **Avanzado**: Agregá un comando `/nick <nombre>` para cambiar tu nombre

## Errores Comunes

### "Address already in use"
El puerto todavía está ocupado. Esperá 1 minuto o usá otro puerto.

### Mensajes se mezclan
Es normal. Los mensajes del otro pueden llegar mientras escribís.

### Ctrl+C no funciona bien
Es porque `fgets()` está bloqueado esperando input. Escribí algo y ahí termina.

## Recursos

- `man socket` - Ver documentación del sistema
- `man pthread_create` - Ver sobre threads
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Tutorial excelente

## Archivos

- `Servidor/servidor.c` - Código del servidor (180 líneas)
- `Cliente/cliente.c` - Código del cliente (175 líneas)
- `Makefile` - Para compilar fácil
- `README.md` - Este archivo

## Notas

- Este es código educativo, simple y directo
- En producción se usarían técnicas más avanzadas (select, epoll, etc.)
- El código tiene manejo básico de errores
- Funciona en Linux, macOS y WSL

---

**¡A experimentar!** 🚀

La mejor forma de aprender es modificar el código y ver qué pasa.
