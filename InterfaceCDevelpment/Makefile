# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Iinclude -IC:/msys64/mingw64/include
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = ../GameServer/DonkeyKong/Client

# Archivos fuente
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Archivo ejecutable
TARGET = $(BIN_DIR)/DonCEyKongJrClient.exe

# Regla principal
all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear directorios si no existen
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Agregar ws2_32 para Winsock
LDLIBS = $(if $(PKG_LIBS),$(PKG_LIBS),-lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32 -mwindows)

# Limpiar archivos compilados
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Recompilar desde cero
rebuild: clean all

# Verificar configuración
check:
	@echo "=== Verificando configuracion ==="
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"

# Ejecutar
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild check run

# Mostrar información
info:
	@echo "Fuentes: $(SOURCES)"
	@echo "Objetos: $(OBJECTS)"