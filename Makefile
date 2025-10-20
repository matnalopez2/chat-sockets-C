# Makefile para Chat con Sockets

CC = gcc
CFLAGS = -Wall -Wextra -pthread
SERVIDOR = Servidor/servidor
CLIENTE = Cliente/cliente

all: servidor cliente
	@echo "✓ Compilado. Para ejecutar:"
	@echo "  Terminal 1: cd Servidor && ./servidor 5000"
	@echo "  Terminal 2: cd Cliente && ./cliente 127.0.0.1 5000"

servidor: $(SERVIDOR)

$(SERVIDOR): Servidor/servidor.c
	$(CC) $(CFLAGS) -o $@ $<

cliente: $(CLIENTE)

$(CLIENTE): Cliente/cliente.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(SERVIDOR) $(CLIENTE)

.PHONY: all servidor cliente clean
