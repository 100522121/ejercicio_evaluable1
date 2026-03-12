CC = gcc
CFLAGS = -Wall -Wextra -fPIC -g
LDFLAGS = -shared
LDLIBS = -lpthread

# Archivos de salida
LIB = libclaves.so
CLIENT = app-cliente

# Objetivo por defecto
all: $(LIB) $(CLIENT)

# Generar la biblioteca dinámica
$(LIB): claves.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compilar el objeto de la biblioteca
claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c $<

# Compilar el cliente enlazado
$(CLIENT): app-cliente.c claves.h $(LIB)
	$(CC) -Wall -Wextra -g -o $@ $< -L. -lclaves -Wl,-rpath,. $(LDLIBS)

# Limpiar archivos temporales
clean:
	rm -f *.o *.so $(CLIENT)

.PHONY: all clean