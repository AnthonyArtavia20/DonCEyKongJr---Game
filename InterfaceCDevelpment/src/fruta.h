

#include "C:/msys64/mingw64/include/raylib.h"

#ifndef FRUTA_H
#define FRUTA_H


typedef struct {
    int activo;
    int liana;
    int puntos;
    Vector2 posicion;
    Rectangle hitbox;
} Fruta;

typedef struct {
    Fruta frutas[50];
    int cantidad_frutas;
} GestorFrutas;

void InicializarFrutas(GestorFrutas* gestor);
int CrearFruta(GestorFrutas* gestor, int liana, float y, int puntos);
void EliminarFruta(GestorFrutas* gestor, int index);
void DibujarFrutas(GestorFrutas* gestor);

#endif

