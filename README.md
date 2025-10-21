# Chat Simple 1:1 con Sockets en C

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-green.svg)](https://www.linux.org/)

Ejemplo simple y didáctico de chat cliente-servidor usando sockets TCP.

**Versión simplificada usando la librería `network.h` de la cátedra.**

## Inicio Rápido

```bash
# 1. Clonar o descargar el proyecto
git clone <url-del-repo>
cd chat-sockets-C

# 2. Compilar
make

# 3. Terminal 1 - Iniciar servidor
cd Servidor
./servidor 5000

# 4. Terminal 2 - Iniciar cliente
cd Cliente
./cliente 127.0.0.1 5000

# 5. Escribir /quit para salir
```

## Tabla de Contenidos

- [Inicio Rápido](#inicio-rápido)
- [Características](#características)
- [¿Qué hace?](#qué-hace)
- [Requisitos](#requisitos)
- [Compilar](#compilar)
- [Usar](#usar)
- [¿Cómo funciona?](#cómo-funciona)
- [Conceptos Clave](#conceptos-clave)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Ejercicios Propuestos](#ejercicios-propuestos)
- [Errores Comunes y Soluciones](#errores-comunes-y-soluciones)
- [Preguntas Frecuentes](#preguntas-frecuentes)
- [Para Seguir Aprendiendo](#para-seguir-aprendiendo)
- [Autor](#autor)
- [Licencia](#licencia)

## Características

✨ **Simple y educativo**: Código claro y fácil de entender  
🔧 **Librería simplificada**: Usa `network.h` para abstraer la complejidad  
💬 **Chat por turnos**: Servidor y cliente alternan mensajes  
🚪 **Salida elegante**: Comando `/quit` para cerrar la conexión  
📝 **Bien documentado**: Comentarios y README completo  
🔄 **Fácil de compilar**: Un simple `make` y listo

## ¿Qué hace?

- **Servidor**: Espera conexiones y chatéa con un cliente (chat por turnos)
- **Cliente**: Se conecta al servidor y chatéa (chat por turnos)

Es un chat **sincrónico**: el servidor recibe y luego envía, el cliente envía y luego recibe.

## Requisitos

- **Sistema Operativo**: Linux, macOS, o WSL (Windows Subsystem for Linux)
- **Compilador**: GCC (GNU Compiler Collection)
- **Herramientas**: Make (opcional, pero recomendado)

Para verificar que tenés GCC instalado:
```bash
gcc --version
```

## Compilar

La forma más fácil es usar el Makefile:

```bash
make
```

Esto compila ambos programas usando la librería `network.h` de la cátedra.

Para ver todos los comandos disponibles:

```bash
make help
```

Comandos del Makefile disponibles:

```bash
make          # Compila servidor y cliente
make servidor # Solo compila el servidor
make cliente  # Solo compila el cliente
make clean    # Elimina archivos compilados
make help     # Muestra ayuda
```

También podés compilar manualmente:

```bash
# Servidor
gcc Servidor/servidor.c util/network.c -o Servidor/servidor -I./util

# Cliente
gcc Cliente/cliente.c util/network.c -o Cliente/cliente -I./util
```

## Usar

### Ejecutar el Chat

**Terminal 1 - Servidor:**
  ```bash
cd Servidor
  ./servidor 5000
  ```

Vas a ver:
```
=== SERVIDOR DE CHAT ===
Puerto: 5000
Esperando cliente...
```

**Terminal 2 - Cliente:**
  ```bash
cd Cliente
  ./cliente 127.0.0.1 5000
  ```

Vas a ver:
```
=== CLIENTE DE CHAT ===
Conectando a 127.0.0.1:5000...
Conectado al servidor!
Escribe '/quit' para salir
─────────────────────────────

Tú: _
```

### Ejemplo de Conversación

**Terminal 1 - Servidor:**
```
=== SERVIDOR DE CHAT ===
Puerto: 5000
Esperando cliente...

Cliente conectado!
Escribe '/quit' para salir
─────────────────────────────

Cliente: Hola servidor!
Tú: Hola cliente! ¿Cómo estás?
Cliente: Muy bien, gracias
Tú: Excelente!
Cliente cerró la conexión.

Servidor cerrado.
```

**Terminal 2 - Cliente:**
```
=== CLIENTE DE CHAT ===
Conectando a 127.0.0.1:5000...
Conectado al servidor!
Escribe '/quit' para salir
─────────────────────────────

Tú: Hola servidor!
Servidor: Hola cliente! ¿Cómo estás?
Tú: Muy bien, gracias
Servidor: Excelente!
Tú: /quit
Cerrando conexión...

Cliente cerrado.
```

### Cómo Salir

Hay 2 formas de cerrar el chat:

1. **Comando /quit** (recomendado):
   ```
   Tú: /quit
   Cerrando conexión...
   ```

2. **Ctrl+C o Ctrl+D**:
   - Presioná Ctrl+C o Ctrl+D
   - Cierra inmediatamente

## ¿Cómo funciona?

### Librería network.h (de la cátedra)

Este proyecto usa funciones simplificadas de la cátedra para manejar sockets:

**Servidor:**
```c
int OpenServer(int port);
```
- Crea socket, hace bind, listen y accept
- **¡Todo en uno!**
- Retorna el file descriptor del socket del cliente conectado

```c
int CloseServer(int sockfd);
```
- Cierra el socket

**Cliente:**
```c
int ConnectToServer(char* ip, int port);
```
- Crea socket y se conecta al servidor
- **¡Todo en uno!**
- Retorna el file descriptor del socket

```c
int DisconnectFromServer(int sockfd);
```
- Cierra el socket

### Funciones que usamos directamente

Después de establecer la conexión con las funciones de la cátedra, usamos:

```c
// Enviar datos
send(sockfd, buffer, strlen(buffer), 0);

// Recibir datos (bloqueante: espera hasta que lleguen)
ssize_t bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
// Retorna: cantidad de bytes recibidos, o 0 si se cerró, o -1 si error
```

### Estructura del Código

**Servidor (servidor.c):**
```
1. Verificar argumentos (puerto)

2. OpenServer(puerto)          ← La librería hace todo esto:
   ├─ socket()                    • Crea socket
   ├─ bind()                      • Asocia con puerto
   ├─ listen()                    • Marca como servidor
   └─ accept()                    • Espera cliente

3. Loop del chat:
   while (1) {
       recv()  → Recibir mensaje del cliente
       printf() → Mostrar mensaje
       fgets() → Leer respuesta del servidor
       send()  → Enviar respuesta al cliente
   }

4. CloseServer(sockfd)         ← Cierra conexión
```

**Cliente (cliente.c):**
```
1. Verificar argumentos (ip, puerto)

2. ConnectToServer(ip, puerto) ← La librería hace todo esto:
   ├─ socket()                    • Crea socket
   ├─ getaddrinfo()               • Resuelve dirección
   └─ connect()                   • Conecta al servidor

3. Loop del chat:
   while (1) {
       fgets() → Leer mensaje del usuario
       send()  → Enviar mensaje al servidor
       recv()  → Recibir respuesta del servidor
       printf() → Mostrar respuesta
   }

4. DisconnectFromServer(sockfd) ← Cierra conexión
```

## Diagrama de Flujo

```
SERVIDOR                             CLIENTE
   |                                    |
OpenServer(puerto)              ConnectToServer(ip, puerto)
   |                                    |
   ├─ socket()                     ├─ socket()
   ├─ bind()                       ├─ getaddrinfo()
   ├─ listen()                     └─ connect() ───────────┐
   └─ accept() <──────────────────────────────────────────┘
   |                                    |
   |                                    |
   ├─ recv() <──────[mensaje]────────── send()
   |                                    |
   ├─ send() ───────[respuesta]──────> recv()
   |                                    |
   └─ ...                               └─ ...
   |                                    |
CloseServer()                    DisconnectFromServer()
```

## Conceptos Clave

### 1. Sockets

Los **sockets** permiten comunicación entre programas por red.

```c
// Crear socket TCP
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//                   IPv4     TCP
```

La librería `network.h` **encapsula** todas estas funciones:
- `socket()` - Crea el endpoint
- `bind()` - Asocia con una dirección/puerto
- `listen()` - Marca como servidor
- `accept()` - Acepta conexiones
- `connect()` - Conecta a un servidor

Nosotros solo llamamos a `OpenServer()` o `ConnectToServer()` y la librería hace todo el trabajo.

### 2. TCP (Transmission Control Protocol)

Es un protocolo **confiable** que:
- ✅ Garantiza que los datos lleguen en orden
- ✅ Garantiza que los datos lleguen sin errores
- ✅ Detecta desconexiones

### 3. Funciones Bloqueantes

Tanto `recv()` como `fgets()` son **bloqueantes**:
- Se quedan esperando hasta que lleguen datos
- El programa "se pausa" en esa línea

```c
recv(sockfd, buffer, BUF_SIZE, 0);  // Espera hasta recibir algo
fgets(buffer, BUF_SIZE, stdin);      // Espera hasta que escribas algo
```

Por eso este chat es **por turnos**: cada lado espera su turno para hablar.

## Flujo de Datos

### Cliente → Servidor

```c
// CLIENTE
fgets(buffer, BUF_SIZE, stdin);        // 1. Leo del teclado
send(sockfd, buffer, strlen(buffer), 0); // 2. Envío por la red
                                        ↓
// SERVIDOR                             ↓
recv(sockfd, buffer, BUF_SIZE, 0);     // 3. Recibo del cliente
printf("Cliente: %s", buffer);         // 4. Muestro en pantalla
```

### Servidor → Cliente

```c
// SERVIDOR
fgets(buffer, BUF_SIZE, stdin);        // 1. Leo del teclado
send(sockfd, buffer, strlen(buffer), 0); // 2. Envío por la red
                                        ↓
// CLIENTE                              ↓
recv(sockfd, buffer, BUF_SIZE, 0);     // 3. Recibo del servidor
printf("Servidor: %s", buffer);        // 4. Muestro en pantalla
```

## Detalles de Implementación

### Manejo de Buffer

```c
char buffer[BUF_SIZE] = {0};

// Limpiar buffer antes de recibir
memset(buffer, 0, BUF_SIZE);

// Asegurar terminación de string
buffer[bytes] = '\0';
```

### Detección de Desconexión

```c
int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);

if (bytes <= 0) {
    // bytes == 0: conexión cerrada
    // bytes < 0: error
    printf("Desconectado.\n");
    break;
}
```

### Comando /quit

```c
if (strncmp(buffer, "/quit", 5) == 0) {
    printf("Cerrando conexión...\n");
    break;
}
```

## Estructura del Proyecto

```
chat-sockets-C/
├── Cliente/
│   └── cliente.c          (96 líneas)
├── Servidor/
│   └── servidor.c         (95 líneas)
├── util/
│   ├── network.h          (11 líneas) - Header de la librería
│   └── network.c          (118 líneas) - Implementación
├── Makefile               (49 líneas)
├── LICENSE                (MIT License)
└── README.md              (este archivo)
```

## Ventajas de usar network.h

| Antes (manual) | Ahora (con network.h) |
|----------------|------------------------|
| `socket()` + `bind()` + `listen()` + `accept()` | `OpenServer(puerto)` |
| `socket()` + `getaddrinfo()` + `connect()` | `ConnectToServer(ip, puerto)` |
| ~150 líneas de código | ~95 líneas de código |
| Manejo complejo de errores | Errores simplificados |

✅ **Menos código**  
✅ **Más simple**  
✅ **Más claro**  
✅ **Mejor para aprender**  

## Ejercicios Propuestos

### Básico
1. Agregá un contador de mensajes enviados/recibidos
2. Agregá timestamp a cada mensaje (usar `time()`)
3. Agregá validación de puerto (1024-65535)

### Intermedio
4. Implementá un comando `/help` que muestre comandos disponibles
5. Agregá colores a los mensajes (usar ANSI escape codes)
6. Implementá un comando `/clear` para limpiar la pantalla

### Avanzado
7. Modificá para que sea asincrónico usando threads
8. Agregá soporte para múltiples clientes (uno a la vez)
9. Implementá un protocolo simple con headers

## Errores Comunes y Soluciones

### 1. "Address already in use"
**Problema:** El puerto todavía está ocupado por una conexión anterior.  
**Solución:** 
- Esperá 1-2 minutos para que el sistema operativo libere el puerto
- O usá otro puerto diferente

```bash
./servidor 5001  # Usar otro puerto
```

Para ver qué proceso está usando el puerto:
```bash
lsof -i :5000  # Linux/macOS
netstat -ano | findstr :5000  # Windows
```

### 2. "Connection refused"
**Problema:** El servidor no está corriendo o no está escuchando en ese puerto.  
**Solución:** 
- Asegurate de iniciar el servidor PRIMERO
- Verificá que el servidor esté escuchando en el puerto correcto
- Verificá que no haya un firewall bloqueando la conexión

### 3. El cliente no puede conectar
**Problema:** IP o puerto incorrectos, o problema de red.  
**Solución:** Verificá que coincidan:
```bash
# Terminal 1 - Servidor
cd Servidor
./servidor 5000

# Terminal 2 - Cliente (debe usar el mismo puerto)
cd Cliente
./cliente 127.0.0.1 5000
```

**Para conexión local:** Usá siempre `127.0.0.1` o `localhost`  
**Para conexión remota:** Usá la IP real de la máquina del servidor

### 4. No se compila / errores de compilación
**Problema:** Faltan dependencias o el compilador no está instalado.  
**Solución:**
```bash
# Instalar GCC en Ubuntu/Debian
sudo apt-get install build-essential

# Instalar GCC en macOS
xcode-select --install

# Verificar instalación
gcc --version
make --version
```

### 5. "Segmentation fault" al ejecutar
**Problema:** Error de puntero o buffer overflow.  
**Solución:** Asegurate de estar usando la versión correcta del código. Si modificaste el código, revisá:
- Límites de arrays
- Punteros nulos
- Llamadas a funciones con parámetros correctos

## Preguntas Frecuentes

### ¿Por qué es por turnos?
Porque no usamos **threads** (hilos). El programa tiene un solo flujo de ejecución que alterna entre enviar y recibir. 

El servidor hace:
1. `recv()` → Espera mensaje del cliente
2. `send()` → Envía respuesta

El cliente hace:
1. `send()` → Envía mensaje
2. `recv()` → Espera respuesta

Para hacer un chat simultáneo (donde ambos puedan escribir al mismo tiempo), necesitarías usar threads o `select()`/`poll()`.

### ¿Puedo hablar con computadoras remotas?
**¡Sí!** Reemplazá `127.0.0.1` con la IP de la otra computadora.

**Ejemplo:**
```bash
# Servidor en la máquina con IP 192.168.1.100
./servidor 5000

# Cliente desde otra máquina
./cliente 192.168.1.100 5000
```

**Requisitos:**
- Ambas máquinas deben estar en la misma red (o tener conectividad)
- El firewall debe permitir la conexión en el puerto elegido
- El servidor debe estar corriendo antes que el cliente se conecte

### ¿Qué pasa si envío mensajes muy largos?
Los mensajes están limitados a `BUF_SIZE` (1024 bytes por defecto). Si enviás un mensaje más largo:
- `fgets()` solo va a leer los primeros 1024 caracteres
- El resto queda en el buffer de entrada para la próxima lectura

Para manejar mensajes más largos necesitarías:
- Aumentar `BUF_SIZE`
- Implementar fragmentación de mensajes
- Usar un protocolo con headers que indique el tamaño total

### ¿Es seguro?
**No.** Los mensajes van en **texto plano** por la red. Cualquiera que intercepte el tráfico puede leer los mensajes.

Para un chat real y seguro necesitarías:
- ✅ **Cifrado TLS/SSL** (como HTTPS para web)
- ✅ **Autenticación** (verificar identidades)
- ✅ **Validación de entrada** (prevenir inyecciones)
- ✅ **Rate limiting** (prevenir abuso)

Este proyecto es **solo educativo** para aprender sockets.

### ¿Puedo usarlo para múltiples clientes?
Actualmente **no**. El servidor acepta un solo cliente a la vez.

Para múltiples clientes necesitarías:
1. **Opción 1 - Threads:** Crear un thread por cada cliente
2. **Opción 2 - Multiplexing:** Usar `select()`, `poll()` o `epoll()`
3. **Opción 3 - Fork:** Crear un proceso hijo por cada cliente

Esto es un ejercicio avanzado propuesto al final del README.

### ¿Funciona en Windows?
**No directamente.** Este código usa sockets POSIX (Linux/Unix).

**Opciones para Windows:**
- ✅ **WSL (Windows Subsystem for Linux)** - Recomendado
- ✅ **Cygwin** - Emula entorno POSIX
- ⚠️ **Winsock** - Requiere reescribir el código para usar la API de Windows

WSL es la forma más fácil y ya está instalado en Windows 10/11 modernos.

## Para Seguir Aprendiendo

### Próximos Pasos
1. **Agregar threads** → Chat simultáneo (no por turnos)
2. **Múltiples clientes** → Servidor que acepta varios clientes
3. **Broadcast** → Mensajes a todos los clientes
4. **Protocolo** → Definir formato de mensajes (JSON, etc)

### Recursos Recomendados

**Documentación del Sistema:**
```bash
man socket    # Documentación de sockets
man recv      # Recepción de datos
man send      # Envío de datos
man bind      # Asociar socket a puerto
man listen    # Marcar socket como servidor
man accept    # Aceptar conexiones
man connect   # Conectar a un servidor
```

**Tutoriales y Guías:**
- 📘 [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - La biblia de programación de redes en C (inglés)
- 📗 [Linux Socket Programming in C](https://www.geeksforgeeks.org/socket-programming-cc/) - Ejemplos prácticos
- 📕 [TCP/IP Sockets in C](https://www.amazon.com/TCP-Sockets-Practical-Programmers-Kaufmann/dp/0128016507) - Libro completo (si querés profundizar)

**Herramientas Útiles:**
```bash
netstat -tuln     # Ver puertos en uso
lsof -i :5000     # Ver qué usa el puerto 5000
tcpdump           # Capturar tráfico de red
wireshark         # Analizar tráfico (GUI)
nc (netcat)       # Herramienta de red versátil
telnet            # Probar conexiones TCP
```

**Temas Relacionados:**
- 🔹 Threads en C (pthread)
- 🔹 Multiplexing (select, poll, epoll)
- 🔹 Protocolos de red (HTTP, FTP, etc.)
- 🔹 Seguridad (TLS/SSL, OpenSSL)
- 🔹 Serialización de datos (JSON, Protocol Buffers)

## Notas Finales

- ✅ **Código educativo**: Simple y directo para aprender
- ✅ **Librería de la cátedra**: Simplifica el manejo de sockets
- ✅ **Manejo básico de errores**: Suficiente para entender
- ✅ **Multiplataforma**: Funciona en Linux, macOS y WSL
- ✅ **Código limpio**: Con comentarios y estructura clara
- ✅ **Fácil de extender**: Base sólida para agregar funcionalidades

Este proyecto es perfecto como **primer contacto con sockets**. Una vez que lo entiendas, podés avanzar a versiones más complejas con threads, múltiples clientes, y protocolos más sofisticados.

---

**¡A experimentar!** 🚀

La mejor forma de aprender es modificar el código y ver qué pasa. Probá romper cosas, arreglarlas, y agregar funcionalidades.

## Autor

**Matías N. López** - 2025

Proyecto educativo para enseñanza de programación en redes con sockets en C.

## Licencia

Este proyecto está bajo la **MIT License**. Ver el archivo `LICENSE` para más detalles.

En resumen: podés usar, modificar y distribuir este código libremente, incluso con fines comerciales.
