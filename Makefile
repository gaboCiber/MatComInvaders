# Definir el compilador
CC = gcc

# Definir las banderas de compilación
CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_mixer)
LDFLAGS = -lncurses $(shell pkg-config --libs sdl2 SDL2_mixer)

# Nombre del ejecutable
TARGET = matcomInvasion

# Archivo fuente
SRC = matcomInvasion.c

# Regla para compilar el programa
$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(LDFLAGS) $(CFLAGS)

# Limpiar archivos compilados
clean:
	rm -f $(TARGET)