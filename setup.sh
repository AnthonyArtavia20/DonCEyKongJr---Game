#!/bin/bash
# Setup script - Configura el entorno y compila ambos proyectos
# Uso: bash setup.sh

set -e  # Salir si hay error

echo "╔════════════════════════════════════════════════════════════╗"
echo "║   Don CEy Kong Jr - Setup Completo                        ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# =====================================================
# 1. CONFIGURAR PATH CON MSYS2
# =====================================================
echo "📋 [1/4] Configurando PATH con MSYS2..."
export PATH="/c/msys64/mingw64/bin:$PATH"

# Verificar que gcc está disponible
if ! command -v gcc &> /dev/null; then
    echo "❌ ERROR: gcc no encontrado. Asegúrate de tener MSYS2 instalado en C:\msys64"
    exit 1
fi
echo "✅ MSYS2 configurado correctamente"
echo ""

# =====================================================
# 2. COMPILAR SERVIDOR JAVA
# =====================================================
echo "📋 [2/4] Compilando Servidor Java..."
cd GameServer

if ! command -v javac &> /dev/null; then
    echo "❌ ERROR: javac no encontrado. Asegúrate de tener Java JDK instalado"
    exit 1
fi

javac -d bin CoreGenericServer/*.java DonkeyKong/Game/*.java DonkeyKong/Server/*.java
echo "✅ Servidor Java compilado"
cd ..
echo ""

# =====================================================
# 3. COMPILAR CLIENTE C
# =====================================================
echo "📋 [3/4] Compilando Cliente C con Raylib..."
cd InterfaceCDevelpment

make clean > /dev/null 2>&1 || true
make

echo "✅ Cliente C compilado"
cd ..
echo ""

# =====================================================
# 4. VERIFICACIÓN FINAL
# =====================================================
echo "📋 [4/4] Verificando archivos compilados..."

if [ -f "GameServer/bin/GameServer/DonkeyKong/Server/DonkeyKongServer.class" ]; then
    echo "✅ Servidor Java: LISTO"
else
    echo "❌ Servidor Java: NO ENCONTRADO"
    exit 1
fi

if [ -f "GameServer/DonkeyKong/Client/DonCEyKongJrClient.exe" ]; then
    echo "✅ Cliente C: LISTO"
else
    echo "❌ Cliente C: NO ENCONTRADO"
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║   ✅ SETUP COMPLETADO EXITOSAMENTE                         ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "🚀 Para ejecutar el proyecto:"
echo ""
echo "   Terminal 1 (Servidor):"
echo "   $ bash run_server.sh"
echo ""
echo "   Terminal 2 (Cliente):"
echo "   $ bash run_client.sh"
echo ""
