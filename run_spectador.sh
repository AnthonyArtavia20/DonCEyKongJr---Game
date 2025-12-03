#!/bin/bash
# run_spectator.sh - Lanza un espectador

NAME="${1:-Espectador}"
ROOM="${2:-1}"

echo "════════════════════════════════════"
echo "  Iniciando ESPECTADOR: $NAME"
echo "  Observando SALA: $ROOM"
echo "════════════════════════════════════"

bash run_client.sh --spectator "$NAME" --room "$ROOM"