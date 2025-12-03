#!/bin/bash
# run_spectador.sh - Lanza un espectador

NAME="${1:-Espectador}"
ROOM="${2:-1}"

echo "════════════════════════════════════"
echo "  Iniciando ESPECTADOR: $NAME"
echo "  Observando SALA: $ROOM"
echo "  (Se observará al Jugador ID: $ROOM)"
echo "════════════════════════════════════"

# Pasar --watch con el mismo ID que la sala
bash run_client.sh --spectator "$NAME" --room "$ROOM" --watch "$ROOM"