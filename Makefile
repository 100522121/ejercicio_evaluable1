CC = gcc
CFLAGS = -Wall -Wextra -fPIC -g
LDFLAGS = -shared
LDLIBS = -lpthread -lrt

# Nombres de las bibliotecas
LIB_LOCAL = libclaves.so
LIB_PROXY = libproxyclaves.so

# Ejecutables
SERVER = servidor_mq
CLIENT_LOCAL = app-cliente
CLIENT_DIST = app-cliente-dist

all: $(LIB_LOCAL) $(LIB_PROXY) $(SERVER) $(CLIENT_LOCAL) $(CLIENT_DIST)

# --- BIBLIOTECAS ---

# Biblioteca local (Apartado A)
$(LIB_LOCAL): claves.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c $<

# Biblioteca Proxy (Apartado B)
$(LIB_PROXY): proxy-mq.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

proxy-mq.o: proxy-mq.c claves.h
	$(CC) $(CFLAGS) -c $<

# --- EJECUTABLES ---

# Servidor (Usa la lógica de libclaves.so)
$(SERVER): servidor-mq.c $(LIB_LOCAL)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,. $(LDLIBS)

# Cliente Monolítico (Apartado A)
$(CLIENT_LOCAL): app-cliente.c $(LIB_LOCAL)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,. $(LDLIBS)

# Cliente Distribuido (Apartado B)
$(CLIENT_DIST): app-cliente.c $(LIB_PROXY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lproxyclaves -Wl,-rpath,. $(LDLIBS)

clean:
	rm -f *.o *.so $(SERVER) $(CLIENT_LOCAL) $(CLIENT_DIST)

.PHONY: all clean
