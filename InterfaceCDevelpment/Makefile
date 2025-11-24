# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Iinclude -IC:/msys64/mingw64/include
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32 -lm

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
all: $(TARGET) copy_dlls

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Copiar DLLs necesarios
copy_dlls: $(BIN_DIR)
	@echo "Copiando DLLs necesarios..."
	@cp -u C:/msys64/mingw64/bin/libraylib.dll $(BIN_DIR)/ 2>/dev/null || true
	@cp -u C:/msys64/mingw64/bin/glfw3.dll $(BIN_DIR)/ 2>/dev/null || true

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear directorios si no existen
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Limpiar archivos compilados (sin borrar assets ni DLLs)
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

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