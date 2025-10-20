# ============================================================================
# Makefile para Chat con Sockets TCP y Threads
# ============================================================================
# Proyecto educativo de programación de sistemas en C
# Compila cliente y servidor de chat 1:1
#
# Uso:
#   make          - Compila ambos programas (cliente y servidor)
#   make all      - Igual que make
#   make servidor - Solo compila el servidor
#   make cliente  - Solo compila el cliente
#   make clean    - Elimina archivos compilados
#   make test     - Ejecuta pruebas básicas de compilación
#   make help     - Muestra esta ayuda
# ============================================================================

# ----------------------------------------------------------------------------
# CONFIGURACIÓN DEL COMPILADOR
# ----------------------------------------------------------------------------

# Compilador C
CC = gcc

# Flags de compilación
# -Wall: habilita la mayoría de advertencias
# -Wextra: advertencias adicionales
# -Werror: trata advertencias como errores (descomenta para modo estricto)
# -std=c11: usar estándar C11
# -pedantic: cumplimiento estricto del estándar
# -pthread: soporte para threads POSIX
CFLAGS = -Wall -Wextra -std=c11 -pedantic -pthread

# Flags de optimización (descomenta una línea según necesites)
# Debug: sin optimización, con símbolos de depuración
CFLAGS_DEBUG = -g -O0 -DDEBUG
# Release: optimización máxima
CFLAGS_RELEASE = -O2 -DNDEBUG

# Por defecto usamos debug para facilitar aprendizaje
CFLAGS += $(CFLAGS_DEBUG)

# Flags del linker
LDFLAGS = -pthread

# ----------------------------------------------------------------------------
# DIRECTORIOS
# ----------------------------------------------------------------------------

SERVIDOR_DIR = Servidor
CLIENTE_DIR = Cliente

# ----------------------------------------------------------------------------
# ARCHIVOS FUENTE Y EJECUTABLES
# ----------------------------------------------------------------------------

SERVIDOR_SRC = $(SERVIDOR_DIR)/servidor.c
CLIENTE_SRC = $(CLIENTE_DIR)/cliente.c

SERVIDOR_BIN = $(SERVIDOR_DIR)/servidor
CLIENTE_BIN = $(CLIENTE_DIR)/cliente

# ----------------------------------------------------------------------------
# REGLAS DE COMPILACIÓN
# ----------------------------------------------------------------------------

# Regla por defecto: compilar todo
.PHONY: all
all: servidor cliente
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  ✓ Compilación completada exitosamente"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "Para ejecutar:"
	@echo "  1. Terminal 1: cd $(SERVIDOR_DIR) && ./servidor 5000"
	@echo "  2. Terminal 2: cd $(CLIENTE_DIR) && ./cliente 127.0.0.1 5000"
	@echo ""

# Compilar solo el servidor
.PHONY: servidor
servidor: $(SERVIDOR_BIN)

$(SERVIDOR_BIN): $(SERVIDOR_SRC)
	@echo "Compilando servidor..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Servidor compilado: $@"

# Compilar solo el cliente
.PHONY: cliente
cliente: $(CLIENTE_BIN)

$(CLIENTE_BIN): $(CLIENTE_SRC)
	@echo "Compilando cliente..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Cliente compilado: $@"

# ----------------------------------------------------------------------------
# REGLAS DE LIMPIEZA
# ----------------------------------------------------------------------------

.PHONY: clean
clean:
	@echo "Limpiando archivos compilados..."
	rm -f $(SERVIDOR_BIN) $(CLIENTE_BIN)
	rm -f $(SERVIDOR_DIR)/*.o $(CLIENTE_DIR)/*.o
	rm -f $(SERVIDOR_DIR)/core $(CLIENTE_DIR)/core
	@echo "✓ Limpieza completada"

# ----------------------------------------------------------------------------
# MODO RELEASE (Optimizado)
# ----------------------------------------------------------------------------

.PHONY: release
release: CFLAGS := -Wall -Wextra -std=c11 -pedantic -pthread $(CFLAGS_RELEASE)
release: clean all
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  ✓ Versión RELEASE compilada (optimizada)"
	@echo "═══════════════════════════════════════════════════════════"

# ----------------------------------------------------------------------------
# REGLAS DE PRUEBA Y VERIFICACIÓN
# ----------------------------------------------------------------------------

.PHONY: test
test: all
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Ejecutando verificaciones básicas..."
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "✓ Verificando existencia de binarios..."
	@test -f $(SERVIDOR_BIN) && echo "  - servidor: OK" || echo "  - servidor: FALLO"
	@test -f $(CLIENTE_BIN) && echo "  - cliente: OK" || echo "  - cliente: FALLO"
	@echo ""
	@echo "✓ Verificando ayuda del servidor..."
	@$(SERVIDOR_BIN) 2>&1 | head -1 || true
	@echo ""
	@echo "✓ Verificando ayuda del cliente..."
	@$(CLIENTE_BIN) 2>&1 | head -1 || true
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Verificación completada"
	@echo "═══════════════════════════════════════════════════════════"

# ----------------------------------------------------------------------------
# ANÁLISIS ESTÁTICO
# ----------------------------------------------------------------------------

.PHONY: check
check:
	@echo "Ejecutando análisis estático con cppcheck..."
	@which cppcheck > /dev/null 2>&1 || (echo "Error: cppcheck no está instalado" && exit 1)
	cppcheck --enable=all --suppress=missingIncludeSystem $(SERVIDOR_SRC) $(CLIENTE_SRC)

# ----------------------------------------------------------------------------
# AYUDA
# ----------------------------------------------------------------------------

.PHONY: help
help:
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Makefile - Chat con Sockets TCP y Threads"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo ""
	@echo "  make             Compila servidor y cliente (modo debug)"
	@echo "  make all         Igual que 'make'"
	@echo "  make servidor    Solo compila el servidor"
	@echo "  make cliente     Solo compila el cliente"
	@echo "  make release     Compila en modo optimizado (release)"
	@echo "  make clean       Elimina archivos compilados"
	@echo "  make test        Verifica que los programas compilen"
	@echo "  make check       Análisis estático con cppcheck"
	@echo "  make help        Muestra esta ayuda"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo ""
	@echo "  1. Compilar todo:"
	@echo "     $$ make"
	@echo ""
	@echo "  2. Ejecutar servidor (en una terminal):"
	@echo "     $$ cd Servidor && ./servidor 5000"
	@echo ""
	@echo "  3. Ejecutar cliente (en otra terminal):"
	@echo "     $$ cd Cliente && ./cliente 127.0.0.1 5000"
	@echo ""
	@echo "  4. Limpiar y recompilar:"
	@echo "     $$ make clean && make"
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""

# ----------------------------------------------------------------------------
# INFORMACIÓN DEL SISTEMA
# ----------------------------------------------------------------------------

.PHONY: info
info:
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Información del Sistema"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "Compilador: $(CC)"
	@$(CC) --version | head -1
	@echo ""
	@echo "Flags de compilación:"
	@echo "  $(CFLAGS)"
	@echo ""
	@echo "Sistema operativo:"
	@uname -s -r
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""

# ----------------------------------------------------------------------------
# INSTALACIÓN (OPCIONAL)
# ----------------------------------------------------------------------------

# Directorio de instalación (modificar según preferencia)
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: install
install: all
	@echo "Instalando binarios en $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(SERVIDOR_BIN) $(BINDIR)/chat-servidor
	install -m 755 $(CLIENTE_BIN) $(BINDIR)/chat-cliente
	@echo "✓ Instalación completada"
	@echo "  - chat-servidor -> $(BINDIR)/chat-servidor"
	@echo "  - chat-cliente -> $(BINDIR)/chat-cliente"

.PHONY: uninstall
uninstall:
	@echo "Desinstalando binarios..."
	rm -f $(BINDIR)/chat-servidor $(BINDIR)/chat-cliente
	@echo "✓ Desinstalación completada"

# ----------------------------------------------------------------------------
# REGLA PARA EVITAR ERRORES
# ----------------------------------------------------------------------------

# Prevenir que Make confunda archivos con objetivos
.SUFFIXES:

