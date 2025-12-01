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

echo "🚀 Servidor escuchando en puerto: $PORT"
echo "👥 Máximo de jugadores: $MAX_PLAYERS"
echo ""
echo "✅ Usando Java 21 específicamente..."
echo ""

# Ruta real del JDK detectado
JAVA_BIN="G:/JDK/jdk21.0.3_9/bin/java.exe"

# Verificar que Java 21 existe
if [ ! -f "$JAVA_BIN" ]; then
    echo "❌ ERROR: No se encuentra Java 21 en:"
    echo "   $JAVA_BIN"
    echo "Por favor verifica la ruta de instalación."
    exit 1
fi

echo "Presiona Ctrl+C para detener el servidor"
echo ""

# Ejecutar servidor con Java 21
"G:/JDK/jdk21.0.3_9/bin/java" -cp bin GameServer.DonkeyKong.Server.DonkeyKongServer $PORT $MAX_PLAYERS


cd ..
