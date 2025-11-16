#include "raylib.h"
#include "mapa.h"
#include <stdio.h>

#define ANCHO_PANTALLA 1200  // Ejemplo de cambio que mencionaste
#define ALTO_PANTALLA 800    // Ejemplo de cambio que mencionaste  
#define TILE_SIZE 17         // Ejemplo de cambio que mencionaste

int main(void) {
    // Inicializar ventana
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Juego con Sistema de Mapas");
    SetTargetFPS(60);
    
    printf("Ventana inicializada: %d x %d\n", ANCHO_PANTALLA, ALTO_PANTALLA);
    
    // Calcular dimensiones del mapa
    int mapaAncho = ANCHO_PANTALLA / TILE_SIZE;
    int mapaAlto = ALTO_PANTALLA / TILE_SIZE;
    
    printf("Dimensiones del mapa: %d x %d (tile size: %d)\n", mapaAncho, mapaAlto, TILE_SIZE);
    
    // Crear mapa
    Mapa *mapa = CrearMapa(mapaAncho, mapaAlto, TILE_SIZE);
    if (!mapa) {
        printf("ERROR: No se pudo crear el mapa\n");
        CloseWindow();
        return -1;
    }
    printf("Mapa creado exitosamente\n");
    
    // Cargar fondo
    CargarFondo(mapa, "assets/Fondo.png");
    printf("Fondo cargado\n");
    
    // Crear mapa de ejemplo
    CrearMapaEjemplo(mapa);
    printf("Mapa de ejemplo creado\n");
    
    printf("Entrando al bucle principal...\n");
    
    // Bucle principal del juego
    while (!WindowShouldClose()) {
        // Dibujado
        BeginDrawing();
            ClearBackground(BLACK);
            
            // Dibujar mapa (incluye el fondo)
            DibujarMapa(mapa);
            
            // Información de debug
            DrawText("Sistema de Mapas - Cuadros invisibles con matriz", 10, 10, 20, WHITE);
            DrawText(TextFormat("Mapa: %dx%d, Tiles: %dx%d", ANCHO_PANTALLA, ALTO_PANTALLA, mapaAncho, mapaAlto), 10, 35, 15, WHITE);
            
        EndDrawing();
    }
    
    // Limpieza
    LiberarMapa(mapa);
    CloseWindow();
    
    printf("Juego terminado correctamente\n");
    return 0;
}