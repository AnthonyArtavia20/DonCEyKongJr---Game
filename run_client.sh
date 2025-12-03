#!/bin/bash
# run_client.sh - Ejecuta el Cliente C/Raylib

export PATH="/c/msys64/mingw64/bin:$PATH"

echo "╔════════════════════════════════════════════════════════════╗"
echo "║   Iniciando Cliente - Don CEy Kong Jr                      ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

cd GameServer/DonkeyKong/Client

if [ ! -f "DonCEyKongJrClient.exe" ]; then
    echo "❌ ERROR: Ejecutable no encontrado"
    exit 1
fi

echo "🎮 Lanzando cliente..."
echo ""

# ✅ Pasar todos los argumentos al ejecutable
./DonCEyKongJrClient.exe "$@"

cd ../../../