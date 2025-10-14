# Chat Cliente-Servidor en C

Proyecto de ejemplo para la materia **Informática I – UTN-FRBA**.  
Este ejercicio muestra cómo implementar un sistema de chat **1 a 1** usando **sockets POSIX** y **hilos (pthread)** en lenguaje C.

---

## 💡 Objetivo

Comprender cómo se comunican dos procesos mediante sockets TCP y cómo usar hilos para lograr comunicación **bidireccional (full-duplex)** entre cliente y servidor.

---

## ⚙️ Requisitos

- Sistema operativo **Linux** (o **WSL** en Windows).
- Compilador **gcc** con soporte para pthreads:
  ```bash
  sudo apt update
  sudo apt install build-essential
  ```
## 🧩 Estructura del proyecto
  ```bash
├── servidor.c     # Código del lado del servidor
├── cliente.c      # Código del lado del cliente
└── README.md
  ```

## 🛠️ Compilación
Desde la terminal del proyecto, ejecutar:
  ```bash
  gcc servidor.c -o servidor -pthread
  gcc cliente.c -o cliente -pthread
  ```
Esto generará los binarios servidor y cliente.

## 🚀 Ejecución
1. Primero iniciar el servidor (en una terminal):
  ```bash
  ./servidor 5000
  ```
Esto deja al servidor escuchando conexiones en el puerto 5000.

2. Luego iniciar el cliente (en otra terminal):
  ```bash
  ./cliente 127.0.0.1 5000
  ```
Si ambos procesos están en máquinas distintas, reemplazá 127.0.0.1 por la IP del servidor.


## 💬 Uso
Escribir un mensaje y presionar Enter para enviarlo.
En ambos extremos se pueden enviar y recibir mensajes simultáneamente.
Para finalizar la conversación, escribir:
  ```bash
  /quit
  ```


## 📘 Conceptos que se ponen en práctica
* Creación de sockets TCP (socket, bind, listen, accept, connect)
* Comunicación mediante send() y recv()
* Control de señales (SIGINT)
* Creación de hilos con pthread_create()
* Sincronización simple mediante variable global running

## 🧠 Ejercicios propuestos
1. Agregar nombre de usuario (/nick) en los mensajes.
2. Mostrar timestamp al recibir cada mensaje.
3. Permitir reconexión del cliente.
4. Ampliar el servidor para aceptar múltiples clientes (uno por hilo).
5. Usar select() o poll() para comunicación sin hilos.

## 🧾 Licencia
Código liberado bajo licencia MIT
Podés usarlo, modificarlo o adaptarlo para tus propios proyectos educativos.


## 👨‍🏫 Autor
Matías Nahuel López

Docente – Informática I, UTN-FRBA

matnalopez.com.ar
