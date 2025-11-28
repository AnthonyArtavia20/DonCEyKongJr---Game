#include "fruta.h"
#include <stdio.h>

void InicializarFrutas(GestorFrutas* gestor) {
    gestor->cantidad_frutas = 0;
    for (int i = 0; i < 50; i++) {
        gestor->frutas[i].activo = 0;
    }
}

int CrearFruta(GestorFrutas* gestor, int liana, float y, int puntos) {
    for (int i = 0; i < 50; i++) {
        if (!gestor->frutas[i].activo) {

            gestor->frutas[i].activo = 1;
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
            printf("[Fruta] Creada en liana %d\n", liana);

            return i;  // devolvemos el índice como “handle”
        }
    }
    return -1;
}

void DibujarFrutas(GestorFrutas* gestor) {
    for (int i = 0; i < 50; i++) {
        if (gestor->frutas[i].activo) {
            DrawCircle(gestor->frutas[i].posicion.x + 10,
                       gestor->frutas[i].posicion.y + 10,
                       10,
                       RED);

            DrawRectangleLinesEx(gestor->frutas[i].hitbox, 1, YELLOW);
        }
    }
}

void EliminarFruta(GestorFrutas* gestor, int index) {
    if (index < 0 || index >= 50) return;

    if (gestor->frutas[index].activo) {
        gestor->frutas[index].activo = 0;
        gestor->cantidad_frutas--;
        printf("[Fruta] Eliminada índice %d\n", index);
    }
}
