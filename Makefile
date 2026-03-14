CC = gcc
CFLAGS = -Wall -Wextra -fPIC -g
LDFLAGS = -shared
LDLIBS = -lpthread -lrt

<<<<<<< HEAD
# Archivos de salida
LIB_CLAVES   = libclaves.so
LIB_PROXY    = libproxyclaves.so
SERVER       = servidor_mq
CLIENT_A     = app-cliente        # Parte A: enlazado con libclaves.so
CLIENT_B     = app-cliente-mq     # Parte B: enlazado con libproxyclaves.so


# Objetivo por defecto: construye todo
all: $(LIB_CLAVES) $(LIB_PROXY) $(SERVER) $(CLIENT_A) $(CLIENT_B)


# PARTE A — Versión no distribuida
$(LIB_CLAVES): claves.o
=======
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
>>>>>>> 2fd846fa24deea620db0f3d117bc378cff35482d
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c $<

<<<<<<< HEAD
$(CLIENT_A): app-cliente.c claves.h $(LIB_CLAVES)
	$(CC) -Wall -Wextra -g -o $@ $< -L. -lclaves -Wl,-rpath,. $(LDLIBS)


# PARTE B — Versión distribuida con colas POSIX
# Biblioteca proxy del lado cliente
$(LIB_PROXY): proxy-mq.o
	$(CC) $(LDFLAGS) -o $@ $^ -lrt

proxy-mq.o: proxy-mq.c claves.h
	$(CC) $(CFLAGS) -c $<

# Ejecutable del servidor
$(SERVER): servidor-mq.c claves.h $(LIB_CLAVES)
	$(CC) -Wall -Wextra -g -o $@ servidor-mq.c -L. -lclaves -Wl,-rpath,. $(LDLIBS) -lrt

$(CLIENT_B): app-cliente.c claves.h $(LIB_PROXY)
	$(CC) -Wall -Wextra -g -o $@ $< -L. -lproxyclaves -Wl,-rpath,. -lrt


# Compilar solo la Parte A
parte-a: $(LIB_CLAVES) $(CLIENT_A)

# Compilar solo la Parte B
parte-b: $(LIB_PROXY) $(SERVER) $(CLIENT_B)


# Limpiar archivos generados
clean:
	rm -f *.o *.so $(CLIENT_A) $(CLIENT_B) $(SERVER)
.PHONY: all parte-a parte-b clean
=======
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
>>>>>>> 2fd846fa24deea620db0f3d117bc378cff35482d
