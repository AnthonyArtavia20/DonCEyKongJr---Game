#ifndef FRUTA_H
#define FRUTA_H

#include "C:/msys64/mingw64/include/raylib.h"

#define MAX_FRUTAS 50
#define MAX_LIANAS 20

typedef struct {
    int id;
    float x_pos;
    int tileY_inicio;
    int tileY_fin;
} LianaInfoFruta;

typedef struct {
    int activo;
    int liana;      // ID de liana
    int puntos;
    Vector2 posicion;
    Rectangle hitbox;
} Fruta;

typedef struct {
    Fruta frutas[MAX_FRUTAS];
    int cantidad_frutas;

    // lianas internas usadas solo por el sistema de frutas
    LianaInfoFruta lianas[MAX_LIANAS];
    int cantidad_lianas;

} GestorFrutas;

// Inicialización
void InicializarFrutas(GestorFrutas* gestor);

// Lianas
void RegistrarLianaFruta(GestorFrutas* gestor, int id, float x_pos,
                         int tileY_inicio, int tileY_fin);
LianaInfoFruta* ObtenerLianaFrutaPorID(GestorFrutas* gestor, int lianaID);

// Frutas
int CrearFruta(GestorFrutas* gestor, int lianaID, float y, int puntos);
void DibujarFrutas(GestorFrutas* gestor);
void EliminarFruta(GestorFrutas* gestor, int index);

#endif
