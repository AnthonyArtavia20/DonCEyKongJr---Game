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
    int id;         // ID único proporcionado por el servidor (o asignado localmente)
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
// CrearFruta ahora recibe un id (proporcionado por servidor o -1 para local)
int CrearFruta(GestorFrutas* gestor, int id, int lianaID, float y, int puntos);
void DibujarFrutas(GestorFrutas* gestor);
void EliminarFruta(GestorFrutas* gestor, int index);

// Eliminar por id (busca y elimina la fruta con ese id)
void EliminarFrutaPorId(GestorFrutas* gestor, int id);

#endif