#!/bin/bash

# ============================================================================
# Script de prueba para el Chat con Sockets
# ============================================================================
# Este script facilita la compilación y prueba del proyecto
#
# Uso:
#   ./test.sh compile   - Compila ambos programas
#   ./test.sh servidor  - Inicia el servidor en puerto 5000
#   ./test.sh cliente   - Conecta un cliente a localhost:5000
#   ./test.sh clean     - Limpia archivos compilados
# ============================================================================

set -e  # Terminar si hay error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuración
PUERTO=${PUERTO:-5000}
IP=${IP:-127.0.0.1}

# ============================================================================
# FUNCIONES
# ============================================================================

print_header() {
    echo -e "${BLUE}"
    echo "═══════════════════════════════════════════════════════════"
    echo "  $1"
    echo "═══════════════════════════════════════════════════════════"
    echo -e "${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

check_dependencies() {
    local missing=0
    
    if ! command -v gcc &> /dev/null; then
        print_error "gcc no está instalado"
        echo "  Instalar con: sudo apt install build-essential"
        missing=1
    fi
    
    return $missing
}

compile() {
    print_header "Compilando Proyecto"
    
    if ! check_dependencies; then
        exit 1
    fi
    
    # Compilar servidor
    echo "Compilando servidor..."
    gcc -Wall -Wextra -std=c11 -pedantic -pthread \
        Servidor/servidor.c -o Servidor/servidor
    print_success "Servidor compilado"
    
    # Compilar cliente
    echo "Compilando cliente..."
    gcc -Wall -Wextra -std=c11 -pedantic -pthread \
        Cliente/cliente.c -o Cliente/cliente
    print_success "Cliente compilado"
    
    echo ""
    print_success "Compilación completada"
}

run_servidor() {
    print_header "Iniciando Servidor"
    print_info "Puerto: $PUERTO"
    print_info "Presiona Ctrl+C para detener"
    echo ""
    
    if [ ! -f "Servidor/servidor" ]; then
        print_error "Servidor no compilado. Ejecuta: ./test.sh compile"
        exit 1
    fi
    
    cd Servidor
    ./servidor "$PUERTO"
}

run_cliente() {
    print_header "Iniciando Cliente"
    print_info "Conectando a: $IP:$PUERTO"
    echo ""
    
    if [ ! -f "Cliente/cliente" ]; then
        print_error "Cliente no compilado. Ejecuta: ./test.sh compile"
        exit 1
    fi
    
    cd Cliente
    ./cliente "$IP" "$PUERTO"
}

clean() {
    print_header "Limpiando archivos compilados"
    
    rm -f Servidor/servidor Cliente/cliente
    rm -f Servidor/*.o Cliente/*.o
    rm -f Servidor/core Cliente/core
    
    print_success "Limpieza completada"
}

run_tests() {
    print_header "Ejecutando Tests Básicos"
    
    # Verificar que existen los archivos fuente
    if [ ! -f "Servidor/servidor.c" ]; then
        print_error "No se encuentra Servidor/servidor.c"
        exit 1
    fi
    
    if [ ! -f "Cliente/cliente.c" ]; then
        print_error "No se encuentra Cliente/cliente.c"
        exit 1
    fi
    
    print_success "Archivos fuente encontrados"
    
    # Compilar
    echo ""
    compile
    
    # Verificar binarios
    echo ""
    print_info "Verificando binarios..."
    
    if [ -x "Servidor/servidor" ]; then
        print_success "Servidor ejecutable"
    else
        print_error "Servidor no es ejecutable"
        exit 1
    fi
    
    if [ -x "Cliente/cliente" ]; then
        print_success "Cliente ejecutable"
    else
        print_error "Cliente no es ejecutable"
        exit 1
    fi
    
    # Probar ayuda
    echo ""
    print_info "Probando mensajes de ayuda..."
    
    if ./Servidor/servidor 2>&1 | grep -q "Uso:"; then
        print_success "Servidor muestra ayuda correctamente"
    else
        print_error "Servidor no muestra ayuda"
    fi
    
    if ./Cliente/cliente 2>&1 | grep -q "Uso:"; then
        print_success "Cliente muestra ayuda correctamente"
    else
        print_error "Cliente no muestra ayuda"
    fi
    
    echo ""
    print_success "Todos los tests pasaron"
}

demo() {
    print_header "Demo Automática"
    print_info "Esta demo abrirá servidor y cliente en terminales separadas"
    echo ""
    
    # Verificar que gnome-terminal o xterm estén disponibles
    if command -v gnome-terminal &> /dev/null; then
        TERM_CMD="gnome-terminal --"
    elif command -v xterm &> /dev/null; then
        TERM_CMD="xterm -e"
    elif command -v konsole &> /dev/null; then
        TERM_CMD="konsole -e"
    else
        print_error "No se encontró emulador de terminal (gnome-terminal, xterm, konsole)"
        print_info "Ejecuta manualmente en terminales separadas:"
        echo "  Terminal 1: ./test.sh servidor"
        echo "  Terminal 2: ./test.sh cliente"
        exit 1
    fi
    
    # Compilar si es necesario
    if [ ! -f "Servidor/servidor" ] || [ ! -f "Cliente/cliente" ]; then
        compile
        echo ""
    fi
    
    # Iniciar servidor
    print_info "Iniciando servidor en nueva terminal..."
    $TERM_CMD bash -c "cd $(pwd) && ./test.sh servidor; exec bash" &
    SERVER_PID=$!
    
    # Esperar a que el servidor inicie
    sleep 2
    
    # Iniciar cliente
    print_info "Iniciando cliente en nueva terminal..."
    $TERM_CMD bash -c "cd $(pwd) && ./test.sh cliente; exec bash" &
    
    print_success "Demo iniciada en nuevas terminales"
    print_info "Cierra las terminales para terminar"
}

show_help() {
    print_header "Ayuda - test.sh"
    
    cat << EOF
Comandos disponibles:

  compile   - Compila servidor y cliente
  servidor  - Inicia el servidor (puerto $PUERTO por defecto)
  cliente   - Conecta un cliente (a $IP:$PUERTO por defecto)
  clean     - Elimina archivos compilados
  test      - Ejecuta tests básicos
  demo      - Demo automática con terminales separadas
  help      - Muestra esta ayuda

Variables de entorno:

  PUERTO    - Puerto a usar (default: 5000)
  IP        - IP del servidor para cliente (default: 127.0.0.1)

Ejemplos:

  # Compilar
  ./test.sh compile

  # Iniciar servidor en puerto 8080
  PUERTO=8080 ./test.sh servidor

  # Conectar cliente a servidor remoto
  IP=192.168.1.100 PUERTO=8080 ./test.sh cliente

  # Demo automática
  ./test.sh demo

Prueba manual:

  Terminal 1: ./test.sh servidor
  Terminal 2: ./test.sh cliente

EOF
}

# ============================================================================
# MAIN
# ============================================================================

case "${1:-help}" in
    compile)
        compile
        ;;
    servidor|server)
        run_servidor
        ;;
    cliente|client)
        run_cliente
        ;;
    clean)
        clean
        ;;
    test|tests)
        run_tests
        ;;
    demo)
        demo
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Comando desconocido: $1"
        echo ""
        show_help
        exit 1
        ;;
esac

