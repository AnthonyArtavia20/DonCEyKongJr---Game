#include "fruta.h"
#include <stdio.h>

void InicializarFrutas(GestorFrutas* gestor) {
    gestor->cantidad_frutas = 0;
    for (int i = 0; i < MAX_FRUTAS; i++) {
        gestor->frutas[i].activo = 0;
        gestor->frutas[i].id = -1;
    }
}

int CrearFruta(GestorFrutas* gestor, int id, int liana, float y, int puntos) {
    for (int i = 0; i < MAX_FRUTAS; i++) {
        if (!gestor->frutas[i].activo) {

            gestor->frutas[i].activo = 1;
            gestor->frutas[i].id = id; // id puede ser -1 si es local/offline
            gestor->frutas[i].liana = liana;
            gestor->frutas[i].puntos = puntos;
            int xPos;

            if (liana == 1){
                xPos = 75;
            }
            else if (liana == 2){
                xPos = 200;
            }
            else if (liana == 3){
                xPos = 300;
            }
            else if (liana == 4){
                xPos = 490;
            }
            else if (liana == 5){
                xPos = 650;
            }
            else if (liana == 6){
                xPos = 750;
            }
            else if (liana == 7){
                xPos = 850;
            }
            else if (liana == 8){
                xPos = 950;
            }
            else if (liana == 9){
                xPos = 1075;
            }
            else {
                xPos = 75; // Default o error
            }
            gestor->frutas[i].posicion = (Vector2){xPos, y};

            gestor->frutas[i].hitbox = (Rectangle){
                xPos, y, 20, 20
            };

            gestor->cantidad_frutas++;
            printf("[Fruta] Creada en liana %d (index=%d, id=%d)\n", liana, i, id);

            return i;  // devolvemos el índice como “handle”
        }
    }
    return -1;
}

void DibujarFrutas(GestorFrutas* gestor) {
    for (int i = 0; i < MAX_FRUTAS; i++) {
        if (gestor->frutas[i].activo) {
            DrawCircle(gestor->frutas[i].posicion.x + 10,
                       gestor->frutas[i].posicion.y + 10,
                       10,
                       RED);

            DrawRectangleLinesEx(gestor->frutas[i].hitbox, 1, YELLOW);
            // Dibujar id (debug)
            #ifdef DEBUG
            DrawText(TextFormat("ID:%d", gestor->frutas[i].id), gestor->frutas[i].posicion.x, gestor->frutas[i].posicion.y - 12, 8, WHITE);
            #endif
        }
    }
}

void EliminarFruta(GestorFrutas* gestor, int index) {
    if (index < 0 || index >= MAX_FRUTAS) return;

    if (gestor->frutas[index].activo) {
        printf("[Fruta] Eliminada índice %d (id=%d)\n", index, gestor->frutas[index].id);
        gestor->frutas[index].activo = 0;
        gestor->frutas[index].id = -1;
        gestor->cantidad_frutas--;
    }
}

void EliminarFrutaPorId(GestorFrutas* gestor, int id) {
    if (id < 0) return;
    for (int i = 0; i < MAX_FRUTAS; i++) {
        if (gestor->frutas[i].activo && gestor->frutas[i].id == id) {
            EliminarFruta(gestor, i);
            return;
        }
    }
    // Si no se encontró, ignorar
}