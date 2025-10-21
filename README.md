# Chat con Sockets y Threads en C

Ejemplo simple de chat cliente-servidor usando sockets TCP y threads POSIX.

**Versión simplificada usando la librería `network.h` de la cátedra.**

## ¿Qué hace?

- **Servidor**: Espera conexiones y chatéa con un cliente
- **Cliente**: Se conecta al servidor y chatéa

Ambos pueden enviar y recibir mensajes **al mismo tiempo** gracias a los threads.

## Compilar

La forma más fácil es usar el Makefile:

```bash
make
```

Esto compila ambos programas usando la librería `network.h` de la cátedra.

También podés compilar manualmente:

  ```bash
# Servidor
gcc Servidor/servidor.c util/network.c -o Servidor/servidor -pthread -I./util

# Cliente
gcc Cliente/cliente.c util/network.c -o Cliente/cliente -pthread -I./util
```

## Usar

### Ver Ayuda

Si ejecutás los programas sin parámetros, te muestran la ayuda:

  ```bash
./Servidor/servidor
```

Muestra:
```
╔═══════════════════════════════════════════════════════════╗
║              🖥️  SERVIDOR DE CHAT - AYUDA                 ║
╚═══════════════════════════════════════════════════════════╝

  📖 USO:
     ./servidor <puerto>

  📝 PARÁMETROS:
     puerto    Número de puerto donde escuchar (1024-65535)

  💡 EJEMPLOS:
     ./servidor 5000
     ./servidor 8080
  ...
```

Lo mismo para el cliente:
  ```bash
./Cliente/cliente
  ```

### Ejecutar el Chat

**Terminal 1 - Servidor:**
  ```bash
cd Servidor
  ./servidor 5000
  ```

Vas a ver algo así:
```
╔═══════════════════════════════════════════════════════════╗
║                🖥️  SERVIDOR DE CHAT                       ║
╚═══════════════════════════════════════════════════════════╝

  📡 Puerto: 5000
  📊 Estado: Esperando cliente...
```

**Terminal 2 - Cliente:**
  ```bash
cd Cliente
  ./cliente 127.0.0.1 5000
  ```

Vas a ver:
```
╔═══════════════════════════════════════════════════════════╗
║                 💬 CLIENTE DE CHAT                        ║
╚═══════════════════════════════════════════════════════════╝

  🔄 Conectando a 127.0.0.1:5000...

╔═══════════════════════════════════════════════════════════╗
║          ✅ CONECTADO AL SERVIDOR - CHAT ACTIVO           ║
╚═══════════════════════════════════════════════════════════╝
  Servidor: 127.0.0.1:5000
  Rol: CLIENTE

  📝 Instrucciones:
     • Escribí tu mensaje y presioná Enter para enviar
     • Escribí /quit para salir
     • Presioná Ctrl+C para cerrar

──────────────────────────────────────────────────────────
✏️  [CLIENTE] Tú > _
```

Ahora escribí mensajes. Cada vez que envíes verás:
```
✏️  [CLIENTE] Tú > Hola!
✅ Enviado
✏️  [CLIENTE] Tú > _
```

Y cuando recibas mensajes:
```
📩 Servidor: ¿Cómo estás?
✏️  [CLIENTE] Tú > _
```

### Cómo Salir

Hay 3 formas de cerrar el chat:

1. **Comando /quit** (recomendado):
   ```
   ✏️  [CLIENTE] Tú > /quit
   👋 Cerrando chat...
   ```

2. **Ctrl+C**:
   - Presioná Ctrl+C
   - Si no cierra inmediatamente, presioná Enter
   - Verás:
   ```
   ⚠️  Ctrl+C detectado - Cerrando...
   💡 Si no cierra inmediatamente, presioná Enter
   ```

3. **Ctrl+D** (EOF):
   - Presioná Ctrl+D en una línea vacía
   - Cierra automáticamente

## ¿Cómo funciona?

### Librería network.h (de la cátedra)

Este proyecto usa funciones simplificadas de la cátedra:

**Servidor:**
- `OpenServer(puerto)` - Crea servidor, hace bind, listen y accept (¡todo en uno!)
- `CloseServer(socket)` - Cierra el servidor

**Cliente:**
- `ConnectToServer(ip, puerto)` - Se conecta al servidor
- `DisconnectFromServer(socket)` - Se desconecta

Estas funciones **simplifican** todo el proceso de sockets que verías normalmente.

### Conceptos Clave

1. **Sockets**: Permiten comunicación entre programas por red
   - La librería encapsula `socket()`, `bind()`, `listen()`, `accept()`, `connect()`
   - Nosotros usamos directamente `send()` / `recv()` para enviar/recibir datos

2. **Threads**: Permiten hacer 2 cosas a la vez
   - Thread 1: Recibe mensajes (con `recv()`)
   - Thread 2: Envía mensajes (con `send()`)
   - Si no usáramos threads, no podríamos escribir mientras esperamos mensajes

3. **TCP**: Protocolo que garantiza que los datos llegan en orden y sin errores

### Estructura del Servidor (simplificada)

```
1. OpenServer(puerto)          ← La librería hace todo esto en 1 paso:
   ├─ socket()                    • Crea socket
   ├─ bind()                      • Asocia con puerto
   ├─ listen()                    • Marca como servidor
   └─ accept()                    • Espera cliente

2. Crear 2 threads:
   - Thread recepción: loop de recv()
   - Thread envío: loop de fgets() + send()

3. Esperar a que terminen los threads

4. CloseServer(socket)         ← Cierra conexión
```

### Estructura del Cliente (simplificada)

```
1. ConnectToServer(ip, puerto) ← La librería hace todo esto en 1 paso:
   ├─ socket()                    • Crea socket
   ├─ getaddrinfo()               • Resuelve dirección
   └─ connect()                   • Conecta al servidor

2. Crear 2 threads:
   - Thread recepción: loop de recv()
   - Thread envío: loop de fgets() + send()

3. Esperar a que terminen los threads

4. DisconnectFromServer(socket) ← Cierra conexión
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
Es normal con I/O concurrente. Los mensajes del otro pueden llegar mientras escribís. No afecta la funcionalidad, solo la presentación visual.

### Ctrl+C ahora funciona correctamente

En esta versión, **Ctrl+C cierra inmediatamente** la conexión.

**¿Cómo lo logramos?**

El signal handler de Ctrl+C hace:
```c
shutdown(global_sockfd, SHUT_RDWR);  // Apaga el socket
close(global_sockfd);                 // Lo cierra
```

Esto **libera** las funciones bloqueantes (`recv()` y `fgets()`), haciendo que los threads terminen inmediatamente.

**Antes** teníamos que presionar Enter porque `fgets()` seguía bloqueado. **Ahora** cerrar el socket desbloquea todo.

## Recursos

- `man socket` - Ver documentación del sistema
- `man pthread_create` - Ver sobre threads
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Tutorial excelente

## Archivos

- `Servidor/servidor.c` - Código del servidor (~240 líneas, simplificado)
- `Cliente/cliente.c` - Código del cliente (~230 líneas, simplificado)
- `util/network.h` - Librería de la cátedra (header)
- `util/network.c` - Librería de la cátedra (implementación)
- `Makefile` - Para compilar fácil
- `README.md` - Este archivo (documentación completa)

## Notas

- Este es código educativo, simple y directo
- Usa la **librería network.h de la cátedra** para simplificar el manejo de sockets
- El código tiene manejo básico de errores
- Funciona en Linux, macOS y WSL
- **Ctrl+C ahora funciona correctamente** cerrando el socket

### Ventajas de usar la librería de la cátedra:

✅ **Menos código** - De ~400 líneas a ~240 líneas  
✅ **Más simple** - No necesitás entender bind/listen/accept en detalle  
✅ **Más claro** - Te enfocás en la lógica del chat, no en sockets  
✅ **Mejor aprendizaje** - Primero entendés el concepto, después los detalles  

---

**¡A experimentar!** 🚀

La mejor forma de aprender es modificar el código y ver qué pasa.
