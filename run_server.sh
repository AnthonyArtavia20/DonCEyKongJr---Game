#!/bin/bash
# run_server.sh - Ejecuta el Servidor Java

cd GameServer

echo "╔════════════════════════════════════════════════════════════╗"
echo "║   Iniciando Servidor Java - Don CEy Kong Jr              ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Puerto y configuración
PORT=5000
MAX_PLAYERS=4

echo " -> Servidor escuchando en puerto: $PORT"
echo " Máximo de jugadores: $MAX_PLAYERS"
echo ""

# RUTA ESPECÍFICA PARA JAVA 21 - MODIFICA ESTA LÍNEA
JAVA_BIN="G:/JDK/jdk21.0.3_9/bin/java.exe"

# Verificar si existe la ruta especificada
if [ ! -f "$JAVA_BIN" ]; then
    echo "❌ ERROR: No se encuentra Java en la ruta especificada"
    echo "Ruta buscada: $JAVA_BIN"
    echo ""
    echo "📋 Soluciones:"
    echo "1. Verifica que Java 21 esté instalado en esa ruta"
    echo "2. Actualiza la variable JAVA_BIN en este script con la ruta correcta"
    echo "3. Instala Java 21 desde: https://adoptium.net/"
    exit 1
fi

# Verificar versión de Java
echo "🔍 Usando Java 21 específico..."
JAVA_VERSION=$("$JAVA_BIN" -version 2>&1 | head -n 1 | cut -d '"' -f2)
echo "✅ Versión de Java: $JAVA_VERSION"

# Verificar que sea versión 21 o superior
VERSION_MAJOR=$(echo "$JAVA_VERSION" | cut -d '.' -f1)
if [ "$VERSION_MAJOR" -lt 21 ]; then
    echo "❌ ERROR: Se requiere Java 21 o superior"
    echo "   Versión encontrada: $JAVA_VERSION"
    exit 1
fi

echo ""
echo "Presiona Ctrl+C para detener el servidor"
echo ""

# Compilar con Java 21 si es necesario
echo "📦 Compilando proyecto con Java 21..."
"$JAVA_BIN" -version 2>&1 | grep version
javac -d bin -cp "bin:../lib/*" $(find . -name "*.java" 2>/dev/null)
if [ $? -ne 0 ]; then
    echo "❌ Error durante la compilación"
    exit 1
fi
echo "✅ Compilación completada"
echo ""

# Ejecutar servidor con Java 21
echo "🚀 Iniciando servidor..."
"$JAVA_BIN" -cp bin GameServer.DonkeyKong.Server.DonkeyKongServer $PORT $MAX_PLAYERS

cd ..