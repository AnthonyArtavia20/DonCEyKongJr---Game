#!/bin/bash
# run_player.sh - Lanza un jugador

NAME="${1:-Jugador$(date +%s)}"

echo "════════════════════════════════════"
echo "  Iniciando JUGADOR: $NAME"
echo "════════════════════════════════════"

bash run_client.sh --player "$NAME"