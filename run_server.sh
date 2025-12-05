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

# Detectar Java automáticamente
echo "🔍 Buscando Java..."
JAVA_BIN=""

# Intentar encontrar java en la ruta del sistema (primero intenta esto)
if command -v java &> /dev/null; then
    JAVA_BIN="java"
    echo "✅ Java encontrado en PATH"
# Si no, intentar rutas conocidas
elif [ -f "G:/JDK/jdk21.0.3_9/bin/java.exe" ]; then
    JAVA_BIN="G:/JDK/jdk21.0.3_9/bin/java.exe"
    echo "✅ Java 21 encontrado en: G:/JDK/jdk21.0.3_9/bin/"
elif [ -f "C:/Program Files/Java/jdk21/bin/java.exe" ]; then
    JAVA_BIN="C:/Program Files/Java/jdk21/bin/java.exe"
    echo "✅ Java 21 encontrado en: C:/Program Files/Java/jdk21/bin/"
elif [ -f "$JAVA_HOME/bin/java.exe" ]; then
    JAVA_BIN="$JAVA_HOME/bin/java.exe"
    echo "✅ Java encontrado en JAVA_HOME: $JAVA_HOME"
else
    echo "❌ ERROR: No se encontró Java en el sistema"
    echo "Por favor instala Java 21+ o verifica que esté en el PATH"
    exit 1
fi

# Verificar versión de Java
JAVA_VERSION=$("$JAVA_BIN" -version 2>&1 | grep -oP 'version "\K[^"]*')
echo "   Versión: $JAVA_VERSION"
echo ""
echo "Presiona Ctrl+C para detener el servidor"
echo ""

# Compilar si es necesario
if [ ! -d "bin" ] || [ -z "$(find bin -name '*.class' 2>/dev/null)" ]; then
    echo "📦 Compilando proyecto..."
    javac -d bin -cp "bin:../lib/*" $(find . -name "*.java" 2>/dev/null | head -20)
    if [ $? -ne 0 ]; then
        echo " Error durante la compilación"
        exit 1
    fi
    echo "Yeeeeah Compilación completada"
    echo ""
fi

# Ejecutar servidor
"$JAVA_BIN" -cp bin GameServer.DonkeyKong.Server.DonkeyKongServer $PORT $MAX_PLAYERS

cd ..
