#!/bin/bash
# run_server.sh - Ejecuta el Servidor Java

export PATH="/c/msys64/mingw64/bin:$PATH"

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
echo "Presiona Ctrl+C para detener el servidor"
echo ""

java -cp bin GameServer.DonkeyKong.Server.DonkeyKongServer $PORT $MAX_PLAYERS

cd ..
