#ifndef MAPA_H
#define MAPA_H

#include "C:/msys64/mingw64/include/raylib.h"

// Definicion de tipos de tiles
typedef enum {
    tile_suelo = 0,
    tile_liana = 1,
    tile_peligro = 2,
    tile_vacio = 3,
    tile_meta = 4
} TileType;

//Estructura del mapa
typedef struct 
{
    int **tiles;  //matriz de tiles
    int ancho; //Ancho en tiles
    int alto; // alto en tiles
    int tileSize; //tamaño de cada tile en pixeles
    Texture2D fondo; //fondo del juego

} Mapa;

//Funciones publicas
Mapa* CrearMapa(int ancho, int alto, int tilesize); //Recibe 3 numeros para formar el mapa del juego
void LiberarMapa(Mapa *mapa);
void DibujarMapa(Mapa *mapa);
void CargarFondo(Mapa *mapa, const char *rutaFondo);
int GetTile(Mapa *mapa, int x, int y);
void SetTile(Mapa *mapa, int x, int y, TileType tipo);

//Funcion para crear un mapa d eejemplo
void CrearMapaEjemplo(Mapa *mapa);

#endif
