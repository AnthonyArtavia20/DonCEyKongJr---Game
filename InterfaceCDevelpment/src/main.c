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
    Vector2 cuadradoPos = {50, 100};
    int cuadradoSize = 50;
    float velocidad = 5.0f;
    bool enSuelo = false;
    bool enAgua = false;
    float velocidadY = 0.0f;      // Nueva: velocidad vertical
    const float gravedad = 0.5f;  // Nueva: fuerza de gravedad
    const float fuerzaSalto = -12.0f; // Nueva: fuerza del salto (negativo porque Y va hacia abajo)
    
    printf("=== INICIANDO BUCLE ===\n");
    
    // Bucle principal
    while (!WindowShouldClose()) {
        // --- MOVIMIENTO HORIZONTAL CON FLECHAS ---
        if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad;
        if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad;
        
        // --- SALTO CON ESPACIO ---
        if (enSuelo && IsKeyPressed(KEY_SPACE)) {
            velocidadY = fuerzaSalto;
            enSuelo = false;
        }
        
        // --- APLICAR GRAVEDAD ---
        velocidadY += gravedad;
        cuadradoPos.y += velocidadY;
        
        // --- DETECCIÓN DE SUELO ---
        enSuelo = HayTileDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enAgua = HayAguaDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        
        // Si detectamos suelo, colocar el cuadrado justo encima y resetear velocidad
        if (enSuelo && velocidadY > 0) {
            // Calcular la posición exacta encima del tile
            int tileY = (int)((cuadradoPos.y + cuadradoSize) / mapa->tileSize);
            cuadradoPos.y = tileY * mapa->tileSize - cuadradoSize;
            velocidadY = 0;
            enSuelo = true;
        }
        if (enAgua) {
            // Si está en agua, reducir la velocidad de caída
            cuadradoPos.x = 50;
            cuadradoPos.y = 600;
        }
        
        // Limites de pantalla
        if (cuadradoPos.x < 0) cuadradoPos.x = 0;
        if (cuadradoPos.x > ANCHO_PANTALLA - cuadradoSize) cuadradoPos.x = ANCHO_PANTALLA - cuadradoSize;
        if (cuadradoPos.y < 0) {
            cuadradoPos.y = 0;
            velocidadY = 0;
        }
        if (cuadradoPos.y > ALTO_PANTALLA - cuadradoSize) {
            cuadradoPos.y = ALTO_PANTALLA - cuadradoSize;
            enSuelo = true;
            velocidadY = 0;
        }
        
        // --- DIBUJADO ---
        BeginDrawing();
            ClearBackground(BLACK);
            
            // Dibujar mapa
            DibujarMapa(mapa);
            
            // Dibujar cuadrado azul movible
            DrawRectangle(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, BLUE);
            DrawRectangleLines(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, YELLOW);
            
            // UI de debug
            DrawText("DEBUG - DON CEY KONG JR", 10, 10, 20, YELLOW);
            DrawText(TextFormat("Posicion: (%.0f, %.0f)", cuadradoPos.x, cuadradoPos.y), 10, 35, 15, GREEN);
            DrawText(TextFormat("En suelo: %s", enSuelo ? "SI" : "NO"), 10, 55, 15, enSuelo ? GREEN : RED);
            DrawText(TextFormat("Velocidad Y: %.1f", velocidadY), 10, 75, 15, BLUE);
            DrawText("Flechas: mover | ESPACIO: saltar", 10, 95, 15, RED);
            DrawText("Presiona ESC para salir", 10, ALTO_PANTALLA - 25, 15, WHITE);
            
        EndDrawing();
    }
    
    LiberarMapa(mapa);
    CloseWindow();
    printf("=== JUEGO TERMINADO ===\n");
    return 0;
} // main