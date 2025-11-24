#ifndef MAPA_H
#define MAPA_H

#include "C:/msys64/mingw64/include/raylib.h"

// Definir TileType aquí
typedef enum {
    tile_vacio,
    tile_suelo, 
    tile_liana,
    tile_peligro,
    tile_meta
} TileType;

// Definir estructura Mapa completamente
typedef struct Mapa {
    int ancho;
    int alto;
    int tileSize;
    int** tiles;
    Texture2D fondo;
} Mapa;

// Funciones del mapa
Mapa* CrearMapa(int ancho, int alto, int tileSize);
void LiberarMapa(Mapa *mapa);
void CargarFondo(Mapa *mapa, const char *rutaFondo);
void DibujarMapa(Mapa *mapa);
int GetTile(Mapa *mapa, int x, int y);
void SetTile(Mapa *mapa, int x, int y, TileType tipo);
void CrearMapaEjemplo(Mapa *mapa);
int HayTileDebajo(Mapa *mapa, float x, float y, int ancho, int alto);
int HayAguaDebajo(Mapa *mapa, float x, float y, int ancho, int alto);
int HayLiana(Mapa *mapa, float x, float y, int ancho, int alto);

#endif