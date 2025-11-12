#include "mapa.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

// Crear un nuevo mapa
Mapa* CrearMapa(int ancho, int alto, int tileSize) {
    // Validaciones básicas
    if (ancho <= 0 || alto <= 0 || tileSize <= 0) {
        TraceLog(LOG_ERROR, "Dimensiones inválidas para el mapa: %dx%d, tileSize: %d", ancho, alto, tileSize);
        return NULL;
    }
    
    Mapa *mapa = (Mapa*)malloc(sizeof(Mapa));
    if (!mapa) {
        TraceLog(LOG_ERROR, "No se pudo asignar memoria para el mapa");
        return NULL;
    }
    
    mapa->ancho = ancho;
    mapa->alto = alto;
    mapa->tileSize = tileSize;
    
    // Reservar memoria para la matriz de tiles
    mapa->tiles = (int**)malloc(alto * sizeof(int*));
    if (!mapa->tiles) {
        TraceLog(LOG_ERROR, "No se pudo asignar memoria para filas del mapa");
        free(mapa);
        return NULL;
    }
    
    for (int i = 0; i < alto; i++) {
        mapa->tiles[i] = (int*)malloc(ancho * sizeof(int));
        if (!mapa->tiles[i]) {
            TraceLog(LOG_ERROR, "No se pudo asignar memoria para columna %d del mapa", i);
            // Liberar memoria ya asignada
            for (int j = 0; j < i; j++) {
                free(mapa->tiles[j]);
            }
            free(mapa->tiles);
            free(mapa);
            return NULL;
        }
        
        // Inicializar todos los tiles como vacíos
        for (int j = 0; j < ancho; j++) {
            mapa->tiles[i][j] = tile_vacio;
        }
    }
    
    // Inicializar la textura del fondo
    mapa->fondo = (Texture2D){0};
    
    TraceLog(LOG_INFO, "Mapa creado: %dx%d, tileSize: %d", ancho, alto, tileSize);
    return mapa;
}

// Liberar memoria del mapa
void LiberarMapa(Mapa *mapa) {
    if (!mapa) return;
    
    // Liberar matriz de tiles
    if (mapa->tiles) {
        for (int i = 0; i < mapa->alto; i++) {
            if (mapa->tiles[i]) {
                free(mapa->tiles[i]);
            }
        }
        free(mapa->tiles);
    }
    
    // Descargar textura del fondo si está cargada
    if (mapa->fondo.id != 0) {
        UnloadTexture(mapa->fondo);
    }
    
    free(mapa);
    TraceLog(LOG_INFO, "Mapa liberado");
}

// Cargar imagen de fondo
void CargarFondo(Mapa *mapa, const char *rutaFondo) {
    if (!mapa) return;
    
    if (mapa->fondo.id != 0) {
        UnloadTexture(mapa->fondo);
    }
    
    // Verificar si el archivo existe
    if (!FileExists(rutaFondo)) {
        TraceLog(LOG_WARNING, "Archivo de fondo no encontrado: %s", rutaFondo);
        return;
    }
    
    // Intentar cargar la imagen
    Image imagen = LoadImage(rutaFondo);
    if (imagen.data != NULL) {
        mapa->fondo = LoadTextureFromImage(imagen);
        UnloadImage(imagen);
        TraceLog(LOG_INFO, "Fondo cargado correctamente: %s (%dx%d)", rutaFondo, mapa->fondo.width, mapa->fondo.height);
    } else {
        TraceLog(LOG_WARNING, "No se pudo cargar el fondo: %s", rutaFondo);
    }
}

// Dibujar el mapa y el fondo
void DibujarMapa(Mapa *mapa) {
    if (!mapa || !mapa->tiles) return;
    
    // Dibujar fondo si está cargado
    if (mapa->fondo.id != 0) {
        DrawTexture(mapa->fondo, 0, 0, WHITE);
    } else {
        // Si no hay fondo, dibujar un fondo simple
        DrawRectangle(0, 0, mapa->ancho * mapa->tileSize, mapa->alto * mapa->tileSize, BLUE);
    }
    
    // Dibujar tiles (para debug)
    for (int y = 0; y < mapa->alto; y++) {
        for (int x = 0; x < mapa->ancho; x++) {
            // Verificar que las coordenadas son válidas
            if (x >= 0 && x < mapa->ancho && y >= 0 && y < mapa->alto) {
                Rectangle tileRect = {
                    (float)(x * mapa->tileSize),
                    (float)(y * mapa->tileSize),
                    (float)mapa->tileSize,
                    (float)mapa->tileSize
                };
                
                // Colores diferentes para cada tipo de tile
                Color tileColor;
                switch (mapa->tiles[y][x]) {
                    case tile_suelo:
                        tileColor = (Color){100, 100, 100, 100};
                        break;
                    case tile_liana:
                        tileColor = (Color){0, 255, 0, 100};
                        break;
                    case tile_peligro:
                        tileColor = (Color){255, 0, 0, 100};
                        break;
                    case tile_meta:
                        tileColor = (Color){255, 255, 0, 100};
                        break;
                    case tile_vacio:
                    default:
                        tileColor = (Color){0, 0, 0, 0};
                        break;
                }
                
                // Solo dibujar tiles no vacíos
                if (mapa->tiles[y][x] != tile_vacio) {
                    DrawRectangleRec(tileRect, tileColor);
                    DrawRectangleLinesEx(tileRect, 1, (Color){255, 255, 255, 50});
                }
            }
        }
    }
}

// Obtener tipo de tile en posición
int GetTile(Mapa *mapa, int x, int y) {
    if (!mapa || !mapa->tiles || x < 0 || x >= mapa->ancho || y < 0 || y >= mapa->alto) {
        return tile_vacio;
    }
    return mapa->tiles[y][x];
}

// Establecer tipo de tile en posición
void SetTile(Mapa *mapa, int x, int y, TileType tipo) {
    if (!mapa || !mapa->tiles || x < 0 || x >= mapa->ancho || y < 0 || y >= mapa->alto) {
        return;
    }
    mapa->tiles[y][x] = tipo;
}

// Crear un mapa de ejemplo
void CrearMapaEjemplo(Mapa *mapa) {
    if (!mapa || !mapa->tiles) return;
    
    TraceLog(LOG_INFO, "Creando mapa de ejemplo: %dx%d", mapa->ancho, mapa->alto);
    
    // Limpiar mapa
    for (int y = 0; y < mapa->alto; y++) {
        for (int x = 0; x < mapa->ancho; x++) {
            mapa->tiles[y][x] = tile_vacio;
        }
    }
    
    // Crear suelo en la parte inferior (solo si el mapa tiene al menos 2 filas)
    if (mapa->alto >= 2) {
        for (int x = 0; x < mapa->ancho; x++) {
            SetTile(mapa, x, mapa->alto - 1, tile_suelo);
            SetTile(mapa, x, mapa->alto - 2, tile_suelo);
        }
    }
    
    // Agregar algunas plataformas (solo si hay espacio)
    if (mapa->ancho >= 10 && mapa->alto >= 5) {
        for (int x = 5; x < 10 && x < mapa->ancho; x++) {
            SetTile(mapa, x, mapa->alto - 5, tile_suelo);
        }
    }
    
    if (mapa->ancho >= 20 && mapa->alto >= 8) {
        for (int x = 15; x < 20 && x < mapa->ancho; x++) {
            SetTile(mapa, x, mapa->alto - 8, tile_suelo);
        }
    }
    
    // Agregar lianas (solo si hay espacio)
    if (mapa->ancho >= 9 && mapa->alto >= 7) {
        SetTile(mapa, 8, mapa->alto - 6, tile_liana);
        SetTile(mapa, 8, mapa->alto - 7, tile_liana);
    }
    
    if (mapa->ancho >= 19 && mapa->alto >= 11) {
        SetTile(mapa, 18, mapa->alto - 9, tile_liana);
        SetTile(mapa, 18, mapa->alto - 10, tile_liana);
    }
    
    // Agregar zona de peligro
    if (mapa->ancho >= 14 && mapa->alto >= 2) {
        for (int x = 12; x < 14 && x < mapa->ancho; x++) {
            SetTile(mapa, x, mapa->alto - 1, tile_peligro);
        }
    }
    
    // Meta (solo si hay espacio)
    if (mapa->ancho >= 3 && mapa->alto >= 3) {
        SetTile(mapa, mapa->ancho - 2, mapa->alto - 2, tile_meta);
    }
    
    TraceLog(LOG_INFO, "Mapa de ejemplo creado exitosamente");
}