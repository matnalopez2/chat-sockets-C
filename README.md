# Chat 1:1 con Sockets TCP y Threads en C

> **Proyecto Educativo**: Implementación de un chat cliente-servidor usando sockets TCP y programación concurrente con threads POSIX.

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard](https://img.shields.io/badge/standard-C11-orange.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![POSIX](https://img.shields.io/badge/POSIX-threads-green.svg)](https://en.wikipedia.org/wiki/POSIX_Threads)

---

## 📋 Tabla de Contenidos

- [Descripción](#-descripción)
- [Conceptos Demostrados](#-conceptos-demostrados)
- [Requisitos](#-requisitos)
- [Compilación](#-compilación)
- [Uso](#-uso)
- [Arquitectura del Código](#-arquitectura-del-código)
- [Conceptos Técnicos Importantes](#-conceptos-técnicos-importantes)
- [Diagramas](#-diagramas)
- [Ejercicios Propuestos](#-ejercicios-propuestos)
- [Preguntas Frecuentes](#-preguntas-frecuentes)
- [Recursos Adicionales](#-recursos-adicionales)

---

## 📖 Descripción

Este proyecto implementa un sistema de chat punto a punto (1:1) usando **sockets TCP** para la comunicación de red y **threads POSIX** para manejar operaciones de lectura y escritura de forma concurrente.

### Características:

✅ **Comunicación full-duplex**: Cliente y servidor pueden enviar y recibir mensajes simultáneamente  
✅ **Programación concurrente**: Usa threads para I/O paralelo (lectura y escritura independientes)  
✅ **Sincronización thread-safe**: Usa mutex para evitar race conditions  
✅ **Manejo robusto de errores**: Validación de entrada y recuperación ante errores  
✅ **Cierre ordenado**: Maneja Ctrl+C, EOF y comando /quit correctamente  
✅ **Código documentado**: Comentarios educativos detallados en español  

---

## 🎓 Conceptos Demostrados

Este proyecto es ideal para aprender:

### 1. **Programación de Red (Sockets)**
- Creación de sockets TCP (`socket()`)
- Conexión cliente-servidor (`bind()`, `listen()`, `accept()`, `connect()`)
- Envío y recepción de datos (`send()`, `recv()`)
- Cierre ordenado de conexiones (`shutdown()`, `close()`)
- Conversión de direcciones (`inet_pton()`, `inet_ntop()`)

### 2. **Programación Concurrente (Threads)**
- Creación y manejo de threads POSIX (`pthread_create()`, `pthread_join()`)
- Sincronización con mutex (`pthread_mutex_t`)
- Comunicación inter-thread segura
- Diseño de aplicaciones multi-threaded

### 3. **Programación de Sistemas**
- Manejo de señales (`signal()`, `SIGINT`, `SIGPIPE`)
- Descriptores de archivo
- I/O bloqueante vs no bloqueante
- Manejo de errores del sistema (`errno`, `perror()`)

### 4. **Buenas Prácticas**
- Validación de entrada
- Liberación de recursos (sockets, mutex, threads)
- Código modular y bien documentado
- Manejo defensivo de errores

---

## 💻 Requisitos

### Sistema Operativo:
- **Linux** (cualquier distribución moderna)
- **macOS** (con herramientas de desarrollo instaladas)
- **Windows WSL** (Windows Subsystem for Linux)

### Herramientas:
- **gcc** (GNU Compiler Collection) versión 4.8 o superior
- **make** (GNU Make)
- Librería **pthread** (incluida en sistemas POSIX)

### Verificar instalación:
```bash
gcc --version
make --version
```

### Instalación de herramientas (si es necesario):

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc make
```

**macOS:**
```bash
xcode-select --install
```

---

## 🔨 Compilación

### Opción 1: Usar el Makefile (recomendado)

```bash
# Compilar ambos programas (servidor y cliente)
make

# Compilar solo el servidor
make servidor

# Compilar solo el cliente
make cliente

# Compilar versión optimizada (release)
make release

# Limpiar archivos compilados
make clean

# Ver ayuda del Makefile
make help
```

### Opción 2: Compilación manual

**Servidor:**
```bash
gcc -Wall -Wextra -std=c11 -pthread Servidor/servidor.c -o Servidor/servidor
```

**Cliente:**
```bash
gcc -Wall -Wextra -std=c11 -pthread Cliente/cliente.c -o Cliente/cliente
```

---

## 🚀 Uso

### Paso 1: Iniciar el Servidor

Abre una terminal y ejecuta:

```bash
cd Servidor
./servidor 5000
```

Donde `5000` es el puerto en el que escuchará (puedes usar cualquier puerto entre 1024-65535).

**Salida esperada:**
```
═══════════════════════════════════════════════════════════
  SERVIDOR DE CHAT - Modo Escucha
═══════════════════════════════════════════════════════════
  Puerto: 5000
  Estado: Esperando conexión de cliente...
  Presiona Ctrl+C para terminar
═══════════════════════════════════════════════════════════
```

### Paso 2: Conectar el Cliente

En **otra terminal**, ejecuta:

```bash
cd Cliente
./cliente 127.0.0.1 5000
```

Donde:
- `127.0.0.1` es la dirección IP del servidor (localhost para pruebas locales)
- `5000` es el puerto del servidor

**Salida esperada:**
```
═══════════════════════════════════════════════════════════
  CLIENTE DE CHAT
═══════════════════════════════════════════════════════════
  Conectando a 127.0.0.1:5000...
  ✓ Conectado exitosamente
═══════════════════════════════════════════════════════════
```

### Paso 3: Chatear

Ahora puedes escribir mensajes en cualquiera de las dos terminales y presionar Enter para enviar.

**Comandos especiales:**
- `/quit` - Cierra la conexión ordenadamente
- `Ctrl+C` - Termina el programa
- `Ctrl+D` - Envía EOF (cierre de conexión)

---

## 📐 Arquitectura del Código

### Estructura del Proyecto

```
chat-sockets-C/
├── Servidor/
│   └── servidor.c        # Código del servidor
├── Cliente/
│   └── cliente.c         # Código del cliente
├── Makefile              # Script de compilación
└── README.md             # Este archivo
```

### Arquitectura del Servidor

```
┌─────────────────────────────────────────────────┐
│              SERVIDOR (servidor.c)              │
├─────────────────────────────────────────────────┤
│                                                 │
│  main()                                         │
│    │                                            │
│    ├──> socket() ────> bind() ──> listen()     │
│    │                                            │
│    ├──> accept() ──────────┐                   │
│    │                        │                   │
│    │              [Conexión establecida]       │
│    │                        │                   │
│    ├──> pthread_create() ──┼─> recv_thread()   │
│    │                        │   (recibe msgs)   │
│    │                        │                   │
│    ├──> pthread_create() ──┼─> send_thread()   │
│    │                        │   (envía msgs)    │
│    │                        │                   │
│    └──> pthread_join() <───┘                   │
│                                                 │
└─────────────────────────────────────────────────┘
```

### Arquitectura del Cliente

```
┌─────────────────────────────────────────────────┐
│              CLIENTE (cliente.c)                │
├─────────────────────────────────────────────────┤
│                                                 │
│  main()                                         │
│    │                                            │
│    ├──> socket() ────> connect()               │
│    │                        │                   │
│    │              [Conexión establecida]       │
│    │                        │                   │
│    ├──> pthread_create() ──┼─> recv_thread()   │
│    │                        │   (recibe msgs)   │
│    │                        │                   │
│    ├──> pthread_create() ──┼─> send_thread()   │
│    │                        │   (envía msgs)    │
│    │                        │                   │
│    └──> pthread_join() <───┘                   │
│                                                 │
└─────────────────────────────────────────────────┘
```

### Flujo de Datos

```
┌─────────────┐                           ┌─────────────┐
│  SERVIDOR   │                           │   CLIENTE   │
├─────────────┤                           ├─────────────┤
│             │                           │             │
│ recv_thread ◄───────── Socket ──────────► send_thread│
│      ▲      │          TCP/IP           │      │      │
│      │      │                           │      ▼      │
│   [mutex]   │                           │   [mutex]   │
│      │      │                           │      │      │
│      ▼      │                           │      ▲      │
│ send_thread ◄───────── Socket ──────────► recv_thread│
│             │                           │             │
└─────────────┘                           └─────────────┘
```

---

## 🧠 Conceptos Técnicos Importantes

### 1. ¿Por qué usamos Threads?

**Problema:** 
- `recv()` y `fgets()` son llamadas **bloqueantes**
- Si esperamos en `recv()`, no podemos escribir mensajes
- Si esperamos en `fgets()`, no podemos recibir mensajes

**Solución:**
- Thread 1: Solo se encarga de **recibir** mensajes (ejecuta `recv()`)
- Thread 2: Solo se encarga de **enviar** mensajes (ejecuta `fgets()` + `send()`)
- Ambos threads corren **simultáneamente** → comunicación full-duplex

### 2. ¿Por qué necesitamos un Mutex?

**Problema:** Race Condition
```c
// Thread 1 (recv_thread)
if (running) {  // Lee 'running'
    // El thread puede ser interrumpido aquí ⚠️
    recv(...);
}

// Thread 2 (send_thread) puede modificar 'running' mientras tanto
running = 0;    // Escribe 'running'
```

**Solución:** Mutex (Mutual Exclusion)
```c
// Thread-safe con mutex
pthread_mutex_lock(&mutex);
int val = running;  // Sección crítica protegida
pthread_mutex_unlock(&mutex);
```

### 3. Diferencia entre `shutdown()` y `close()`

| Función | Qué hace |
|---------|----------|
| `shutdown(fd, SHUT_WR)` | Cierra solo la escritura (half-close). El peer recibe EOF pero aún podemos leer respuestas. |
| `close(fd)` | Cierra completamente el socket (lectura y escritura). |

### 4. ¿Qué es Full-Duplex?

**Half-Duplex:** Solo uno puede hablar a la vez (como walkie-talkies 📻)  
**Full-Duplex:** Ambos pueden hablar simultáneamente (como teléfonos 📞)

Este chat es **full-duplex** gracias a los threads independientes.

### 5. Endianness y Network Byte Order

```c
htons(port);  // Host TO Network Short (convierte endianness)
ntohs(port);  // Network TO Host Short (convierte de vuelta)
```

**¿Por qué?**
- Diferentes CPUs guardan números de forma diferente (little-endian vs big-endian)
- La red usa **big-endian** (network byte order) como estándar
- `htons/htonl` aseguran compatibilidad entre diferentes arquitecturas

---

## 📊 Diagramas

### Ciclo de Vida de una Conexión TCP

```
SERVIDOR                                    CLIENTE

socket()                                    socket()
  │                                           │
bind()                                        │
  │                                           │
listen()                                      │
  │                                           │
accept() ◄─────── [SYN] ──────────────────── connect()
  │       ────── [SYN-ACK] ────────────────►   │
  │       ◄────── [ACK] ──────────────────────  │
  │                                           │
  │◄══════════ CONEXIÓN ESTABLECIDA ═══════►│
  │                                           │
send() ───────── datos ──────────────────► recv()
  │                                           │
recv() ◄────────── datos ──────────────── send()
  │                                           │
close() ◄─────── [FIN] ──────────────────── close()
        ────── [FIN-ACK] ────────────────►
```

### Estados de los Threads

```
             [Inicio del Programa]
                     │
         ┌───────────┴───────────┐
         ▼                       ▼
    recv_thread()           send_thread()
         │                       │
         ├─► Bloqueado en        ├─► Bloqueado en
         │   recv()              │   fgets()
         │                       │
         │   [Llega mensaje]     │   [Usuario escribe]
         ▼                       ▼
     printf(mensaje)         send(mensaje)
         │                       │
         └───────────┬───────────┘
                     │
              [Uno termina]
                     │
              set_running(0)
                     │
       ┌─────────────┴─────────────┐
       ▼                           ▼
  El otro thread                   │
  detecta !is_running()            │
  y termina                        │
       │                           │
       └───────────┬───────────────┘
                   ▼
            pthread_join()
                   │
                   ▼
         [Programa termina]
```

---

## 🎯 Ejercicios Propuestos

### Nivel Básico:

1. **Modificar el prompt:**
   - Cambia `[Tú] >` por `[Usuario] >` o tu nombre

2. **Agregar timestamps:**
   - Imprime la hora junto a cada mensaje recibido
   - Pista: usa `time()` y `strftime()`

3. **Limitar tamaño de mensaje:**
   - Rechaza mensajes que excedan cierto tamaño
   - Muestra un mensaje de error al usuario

### Nivel Intermedio:

4. **Soporte para múltiples clientes:**
   - Modifica el servidor para aceptar más de un cliente
   - Usa un thread por cliente
   - Pista: crea un bucle que llame a `accept()` repetidamente

5. **Broadcast de mensajes:**
   - Cuando un cliente envía un mensaje, envíalo a todos los demás
   - Mantén una lista de sockets conectados

6. **Comando /help:**
   - Implementa un comando que muestre los comandos disponibles
   - Procesa el comando localmente (sin enviarlo)

### Nivel Avanzado:

7. **Nombres de usuario:**
   - Al conectar, pide al cliente su nombre
   - Muestra el nombre junto a cada mensaje

8. **Mensajes privados:**
   - Implementa `/msg <usuario> <mensaje>` para enviar mensajes privados

9. **Persistencia:**
   - Guarda el historial de mensajes en un archivo
   - Carga el historial al iniciar

10. **Encriptación:**
    - Encripta los mensajes antes de enviarlos
    - Pista: investiga XOR cipher o librerías como OpenSSL

---

## ❓ Preguntas Frecuentes

### P: ¿Por qué obtengo "Address already in use"?

**R:** El puerto aún está en estado TIME_WAIT del sistema operativo. Opciones:
1. Espera 1-2 minutos
2. Usa un puerto diferente
3. El código ya incluye `SO_REUSEADDR` para mitigar esto

### P: ¿Puedo probar esto en redes diferentes (no localhost)?

**R:** Sí, pero:
1. Asegúrate de que el firewall permita el puerto
2. Usa la IP real del servidor (no 127.0.0.1)
3. Verifica que ambos estén en la misma red o usa port forwarding

### P: ¿Por qué los mensajes se mezclan a veces?

**R:** Es normal con I/O concurrente. El prompt puede ser interrumpido por un mensaje entrante. Esto no afecta la funcionalidad.

### P: ¿Cómo depuro el programa?

**R:** 
```bash
# Compilar con símbolos de depuración (el Makefile ya lo hace)
make clean && make

# Ejecutar con gdb
gdb ./Servidor/servidor
(gdb) run 5000
```

### P: ¿Funciona en Windows?

**R:** No directamente. Windows usa Winsock en lugar de BSD sockets. Opciones:
1. Usa WSL (Windows Subsystem for Linux) ✅
2. Adapta el código para Winsock (requiere cambios significativos)

---

## 📚 Recursos Adicionales

### Libros:
- **"Unix Network Programming"** - W. Richard Stevens (la biblia de sockets)
- **"The Linux Programming Interface"** - Michael Kerrisk
- **"Programming with POSIX Threads"** - David R. Butenhof

### Man Pages (documentación del sistema):
```bash
man socket      # Crear sockets
man bind        # Asociar dirección a socket
man listen      # Marcar socket como pasivo
man accept      # Aceptar conexión
man connect     # Conectar a servidor
man send        # Enviar datos
man recv        # Recibir datos
man pthread_create  # Crear thread
man pthread_join    # Esperar thread
man pthread_mutex   # Mutex
```

### Tutoriales Online:
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Excelente introducción a sockets
- [POSIX Threads Programming (LLNL)](https://hpc-tutorials.llnl.gov/posix/) - Tutorial de threads

### Herramientas de Depuración:
```bash
# Ver conexiones de red activas
netstat -tulnp | grep 5000

# Monitorear tráfico de red
tcpdump -i lo port 5000

# Analizar con Wireshark
wireshark

# Verificar threads activos
ps -T -p <PID>
```

---

## 👨‍🏫 Notas para Profesores

### Objetivos de Aprendizaje:

Al completar este proyecto, los estudiantes deberían poder:
- ✅ Explicar el modelo cliente-servidor
- ✅ Crear aplicaciones de red básicas con sockets TCP
- ✅ Implementar concurrencia con threads POSIX
- ✅ Identificar y evitar race conditions
- ✅ Manejar recursos del sistema correctamente (no fugas de memoria)

### Sugerencias de Evaluación:

1. **Código:** Evaluar calidad, estilo y documentación
2. **Funcionalidad:** Verificar que compile sin warnings y funcione correctamente
3. **Extensiones:** Pedir implementar uno o más ejercicios propuestos
4. **Preguntas teóricas:** 
   - ¿Qué es un socket?
   - ¿Por qué usamos threads?
   - ¿Qué es un race condition?
   - Diferencia entre TCP y UDP

### Variaciones del Proyecto:

- Implementar con UDP en lugar de TCP
- Añadir interfaz gráfica (GTK, Qt)
- Versión en C++ usando clases
- Implementar protocolo personalizado sobre TCP

---

## 📝 Licencia

Este es un proyecto educativo de código abierto. Siéntete libre de usarlo, modificarlo y compartirlo para fines académicos.

---

## 🤝 Contribuciones

Este proyecto fue creado con fines educativos. Mejoras y sugerencias son bienvenidas:

1. Mejoras en documentación
2. Correcciones de bugs
3. Ejercicios adicionales
4. Traducciones a otros idiomas

---

## ✨ Créditos

**Proyecto:** Chat con Sockets y Threads  
**Propósito:** Educativo - Programación de Sistemas  
**Lenguaje:** C (estándar C11)  
**Plataforma:** POSIX (Linux, macOS, BSD)  

---

**¡Feliz aprendizaje! 🚀**

Si tienes preguntas o encuentras problemas, revisa la sección de [Preguntas Frecuentes](#-preguntas-frecuentes) o consulta los [Recursos Adicionales](#-recursos-adicionales).
