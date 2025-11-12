# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g

# Rutas para raylib (w64devkit)
RAYLIB_PATH = C:/raylib/w64devkit
INCLUDE_PATHS = -Iinclude -I$(RAYLIB_PATH)/include
LIBRARY_PATHS = -L$(RAYLIB_PATH)/lib

# Librerías a enlazar
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Archivos fuente y objeto - asegúrate de que encuentra todos los .c
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Archivo ejecutable
TARGET = $(BIN_DIR)/mi_juego.exe

# Regla principal
all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBRARY_PATHS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c $< -o $@

# Crear directorios si no existen (comandos Unix para Git Bash)
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Limpiar archivos compilados (comandos Unix para Git Bash)
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Recompilar desde cero
rebuild: clean all

# Verificar configuración
check:
	@echo "=== Verificando configuracion ==="
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "RAYLIB_PATH: $(RAYLIB_PATH)"

.PHONY: all clean rebuild check