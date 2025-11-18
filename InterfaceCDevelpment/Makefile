# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g

# Rutas para raylib (w64devkit) - se usa como respaldo si no existe pkg-config
RAYLIB_PATH = C:/raylib/w64devkit

# Intentar usar pkg-config (si está instalado en el entorno MinGW64/MSYS2)
PKG_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)

# Include y library flags: usar pkg-config si está disponible, si no usar la ruta fija
INCLUDE_PATHS = -Iinclude $(PKG_CFLAGS)
LIBRARY_PATHS = $(if $(PKG_LIBS),, -L$(RAYLIB_PATH)/lib)

# Librerías a enlazar (fallback si pkg-config no retorna nada)
LDLIBS = $(if $(PKG_LIBS),$(PKG_LIBS),-lraylib -lopengl32 -lgdi32 -lwinmm -mwindows)

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = ..\GameServer\DonkeyKong\Client

# Archivos fuente y objeto - asegúrate de que encuentra todos los .c
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Archivo ejecutable
TARGET = $(BIN_DIR)/DonCEyKongJrClient.exe

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

# Mostrar flags de compilacion/enlace para depuracion
showflags:
	@echo "CFLAGS: $(CFLAGS) $(INCLUDE_PATHS)"
	@echo "LDFLAGS: $(LIBRARY_PATHS) $(LDLIBS)"
	@echo "PKG_CFLAGS: $(PKG_CFLAGS)"
	@echo "PKG_LIBS: $(PKG_LIBS)"

# Ejecutar en modo debug: recompila forzando subsistema consola, copia DLLs y vuelca salida a run.log
run-debug: clean
	@echo "== Recompilando con subsistema console (debug) =="
	$(MAKE) all LDLIBS="-lraylib -Wl,-subsystem,console"
	@echo "== Copiando libraylib dll a $(BIN_DIR) (si existe) =="
	@cp /mingw64/bin/libraylib*.dll $(BIN_DIR)/ 2>/dev/null || true
	@echo "== Ejecutando y guardando salida en run.log =="
	@./$(TARGET) > run.log 2>&1 || echo "Program exited with code: $$?"
	@echo "--- run.log (first 200 lines) ---"
	@sed -n '1,200p' run.log || true
	@echo "--- fin run.log ---"

.PHONY: run-debug showflags