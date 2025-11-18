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
    bool enLiana = false;
    bool sujetandoLiana = false;  // Nueva: control manual de sujeción
    float velocidadY = 0.0f;
    const float gravedad = 0.5f;
    const float fuerzaSalto = -12.0f;
    
    printf("=== INICIANDO BUCLE ===\n");
    
    // Bucle principal
    while (!WindowShouldClose()) {
        // --- MOVIMIENTO HORIZONTAL CON FLECHAS ---
        if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad;
        if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad;
        
        // --- DETECCIÓN DE ESTADOS ---
        enSuelo = HayTileDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enAgua = HayAguaDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enLiana = HayLiana(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        
        // --- CONTROL MANUAL DE LIANAS (Tecla Z para agarrar/soltar) ---
        if (enLiana && IsKeyPressed(KEY_Z)) {
            sujetandoLiana = !sujetandoLiana; // Alternar entre agarrar y soltar
        }
        
        // --- SALTO CON ESPACIO ---
        if ((enSuelo || sujetandoLiana) && IsKeyPressed(KEY_SPACE)) {
            velocidadY = fuerzaSalto;
            enSuelo = false;
            sujetandoLiana = false; // Soltar liana al saltar
        }
        
        // --- COMPORTAMIENTO EN LIANA (solo si está sujetando) ---
        if (sujetandoLiana && enLiana) {
            // Permitir subir y bajar en la liana
            if (IsKeyDown(KEY_UP)) {
                cuadradoPos.y -= velocidad;
            }
            else if (IsKeyDown(KEY_DOWN)) {
                cuadradoPos.y += velocidad;
            }
            
            // Movimiento horizontal reducido en liana
            if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad * 0.3f;
            if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad * 0.3f;
            
            // Anular gravedad mientras esté sujetando
            velocidadY = 0;
        }
        else {
            // --- COMPORTAMIENTO NORMAL (no sujetando liana) ---
            sujetandoLiana = false; // Asegurar que no está sujetando
            
            // Aplicar gravedad
            velocidadY += gravedad;
            cuadradoPos.y += velocidadY;
        }
        
        // --- COMPORTAMIENTO EN AGUA ---
        if (enAgua) {
            cuadradoPos.x = 50;
            cuadradoPos.y = 600;
            velocidadY = 0;
            sujetandoLiana = false; // Soltar liana si cae al agua
        }
        
        // --- CORRECCIÓN DE COLISIÓN CON SUELO ---
        if (enSuelo && velocidadY > 0) {
            int tileY = (int)((cuadradoPos.y + cuadradoSize) / mapa->tileSize);
            cuadradoPos.y = tileY * mapa->tileSize - cuadradoSize;
            velocidadY = 0;
            enSuelo = true;
            sujetandoLiana = false; // Soltar liana al tocar suelo
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
            velocidadY = -1;
        }
        
        // --- DIBUJADO ---
        BeginDrawing();
            ClearBackground(BLACK);
            
            // Dibujar mapa
            DibujarMapa(mapa);
            
            // Dibujar cuadrado (color según estado)
            Color colorCuadrado = BLUE;
            if (enLiana && !sujetandoLiana) colorCuadrado = ORANGE;    // Liana disponible
            if (sujetandoLiana) colorCuadrado = GREEN;                 // Sujetando liana
            if (enAgua) colorCuadrado = SKYBLUE;                       // En agua
            
            DrawRectangle(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, colorCuadrado);
            DrawRectangleLines(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, YELLOW);
            
            // UI de debug
            DrawText("DEBUG - DON CEY KONG JR", 10, 10, 20, YELLOW);
            DrawText(TextFormat("Posicion: (%.0f, %.0f)", cuadradoPos.x, cuadradoPos.y), 10, 35, 15, GREEN);
            DrawText(TextFormat("En suelo: %s", enSuelo ? "SI" : "NO"), 10, 55, 15, enSuelo ? GREEN : RED);
            DrawText(TextFormat("Liana disponible: %s", enLiana ? "SI" : "NO"), 10, 75, 15, enLiana ? ORANGE : RED);
            DrawText(TextFormat("Sujetando liana: %s", sujetandoLiana ? "SI" : "NO"), 10, 95, 15, sujetandoLiana ? GREEN : RED);
            DrawText(TextFormat("En agua: %s", enAgua ? "SI" : "NO"), 10, 115, 15, enAgua ? BLUE : RED);
            DrawText("Flechas: mover | ESPACIO: saltar | Z: agarrar/soltar liana", 10, 135, 12, RED);
            DrawText("Presiona ESC para salir", 10, ALTO_PANTALLA - 25, 15, WHITE);
            
        EndDrawing();
    }
    
    LiberarMapa(mapa);
    CloseWindow();
    printf("=== JUEGO TERMINADO ===\n");
    return 0;
} // main