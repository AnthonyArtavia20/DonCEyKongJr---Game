#include "C:/msys64/mingw64/include/raylib.h"
#include "mapa.h"
#include <stdio.h>

#define ANCHO_PANTALLA 1200
#define ALTO_PANTALLA 800
#define TILE_SIZE 17

int main(void) {
    // Inicializar ventana
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Don CEy Kong Jr - Debug");
    SetTargetFPS(60);
    
    printf("=== INICIANDO DEBUG ===\n");
    printf("Pantalla: %dx%d, Tile: %d\n", ANCHO_PANTALLA, ALTO_PANTALLA, TILE_SIZE);
    
    // Calcular dimensiones del mapa
    int mapaAncho = ANCHO_PANTALLA / TILE_SIZE;
    int mapaAlto = ALTO_PANTALLA / TILE_SIZE;
    printf("Mapa calculado: %dx%d tiles\n", mapaAncho, mapaAlto);
    
    // Crear mapa
    Mapa *mapa = CrearMapa(mapaAncho, mapaAlto, TILE_SIZE);
    if (!mapa) {
        printf("ERROR: Mapa NULL\n");
        CloseWindow();
        return -1;
    }
    printf("Mapa creado: %dx%d\n", mapa->ancho, mapa->alto);
    
    // Verificar archivo de fondo
    if (!FileExists("assets/Fondo.png")) {
        printf("AVISO: assets/Fondo.png no existe - usando fondo azul\n");
    }
    
    // Cargar fondo y crear ejemplo
    CargarFondo(mapa, "assets/Fondo.png");
    CrearMapaEjemplo(mapa);
    printf("Mapa ejemplo creado\n");
    
    // --- VARIABLES DEL CUADRADO AZUL ---
    Vector2 cuadradoPos = {50, ALTO_PANTALLA - 100};
    int cuadradoSize = 50;
    float velocidad = 5.0f;
    
    printf("=== INICIANDO BUCLE ===\n");
    
    // Bucle principal
    while (!WindowShouldClose()) {
        // --- MOVIMIENTO CON FLECHAS ---
        if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad;
        if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad;
        if (IsKeyDown(KEY_UP)) cuadradoPos.y -= velocidad;
        if (IsKeyDown(KEY_DOWN)) cuadradoPos.y += velocidad;
        
        // Limites de pantalla
        if (cuadradoPos.x < 0) cuadradoPos.x = 0;
        if (cuadradoPos.x > ANCHO_PANTALLA - cuadradoSize) cuadradoPos.x = ANCHO_PANTALLA - cuadradoSize;
        if (cuadradoPos.y < 0) cuadradoPos.y = 0;
        if (cuadradoPos.y > ALTO_PANTALLA - cuadradoSize) cuadradoPos.y = ALTO_PANTALLA - cuadradoSize;
        
        // --- DIBUJADO ---
        BeginDrawing();
            ClearBackground(BLACK);
            
            // Dibujar mapa
            DibujarMapa(mapa);
            
            // Dibujar cuadrado azul movible
            DrawRectangle(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, BLUE);
            
            // UI de debug
            DrawText("DEBUG - DON CEY KONG JR", 10, 10, 20, YELLOW);
            DrawText(TextFormat("Posicion: (%.0f, %.0f)", cuadradoPos.x, cuadradoPos.y), 10, 35, 15, GREEN);
            DrawText("Flechas para mover el cuadrado azul", 10, 55, 15, RED);
            DrawText("Presiona ESC para salir", 10, ALTO_PANTALLA - 25, 15, BLACK);
            
        EndDrawing();
    }
    
    LiberarMapa(mapa);
    CloseWindow();
    printf("=== JUEGO TERMINADO ===\n");
    return 0;
}