#!/usr/bin/env bash
# run_observador.sh
# Lanza un observador y le pregunta qué partida (1 o 2) quiere ver.
# Mapea:
#   1 -> puerto 5000
#   2 -> puerto 5001
# Uso: ./run_observador.sh [NombreObservador] [ruta_al_binario_opcional]
# Si no pasas ruta al binario intentará encontrarlo en rutas comunes.
set -euo pipefail

NAME="${1:-Observador}"
CUSTOM_BIN="${2:-}"

# Preguntar al usuario qué partida quiere ver (1 o 2)
echo "¿Qué partida quieres observar?"
echo "  1) Partida 1 (puerto 5000)"
echo "  2) Partida 2 (puerto 5001)"
read -rp "Elige 1 o 2: " CHOICE

if [[ "$CHOICE" != "1" && "$CHOICE" != "2" ]]; then
  echo "Opción inválida: $CHOICE. Ejecuta el script de nuevo y elige 1 o 2."
  exit 1
fi

if [[ "$CHOICE" == "1" ]]; then
  PORT=5000
elif [[ "$CHOICE" == "2" ]]; then
  PORT=5001
fi

# Determinar ejecutable del cliente
if [[ -n "$CUSTOM_BIN" ]]; then
  CLIENT_BIN="$CUSTOM_BIN"
else
  SEARCH_PATHS=(
    "./client"
    "./client.exe"
    "./InterfaceCDevelpment/client"
    "./InterfaceCDevelpment/client.exe"
    "./InterfaceCDevelpment/DonCEyKongJrClient.exe"
    "./GameServer/DonkeyKong/Client/DonCEyKongJrClient.exe"
  )
  CLIENT_BIN=""
  for p in "${SEARCH_PATHS[@]}"; do
    if [[ -x "$p" ]]; then
      CLIENT_BIN="$p"
      break
    fi
  done
  if [[ -z "$CLIENT_BIN" ]]; then
    CLIENT_BIN="$(find . -maxdepth 6 -type f \( -iname '*client*' -o -iname '*DonCEyKong*' \) -print -quit 2>/dev/null || true)"
  fi
fi

if [[ -z "$CLIENT_BIN" ]]; then
  echo "No se encontró el ejecutable del cliente."
  echo "Pasa la ruta al binario como segundo parámetro, por ejemplo:"
  echo "  $0 Observador ./InterfaceCDevelpment/client.exe"
  exit 1
fi

# Hacer ruta absoluta
CLIENT_BIN_ABS="$(cd "$(dirname "$CLIENT_BIN")" 2>/dev/null && pwd)/$(basename "$CLIENT_BIN")"
if [[ ! -f "$CLIENT_BIN_ABS" ]]; then
  echo "ERROR: archivo no encontrado: $CLIENT_BIN_ABS"
  exit 1
fi

# Asegurar ejecutable (no falla si no aplica)
chmod +x "$CLIENT_BIN_ABS" 2>/dev/null || true

# Cambiar al directorio del binario para que las rutas relativas (assets/) funcionen
BIN_DIR="$(dirname "$CLIENT_BIN_ABS")"
echo "Cambiando a directorio: $BIN_DIR"
cd "$BIN_DIR"

echo "Iniciando observador '$NAME' en la partida $CHOICE (puerto $PORT)..."
# Ejecutar el cliente como espectador apuntando al puerto de la partida elegida
# Nota: el cliente usa --spectator <name> --port <port>
exec "$CLIENT_BIN_ABS" --spectator "$NAME" --port "$PORT"