#include "C:/msys64/mingw64/include/raylib.h"
#include "mapa.h"
#include "socket_client.h"
#include <stdio.h>

#define ANCHO_PANTALLA 1200
#define ALTO_PANTALLA 800
#define TILE_SIZE 17

int main(void) {
    // Inicializar ventana
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Don CEy Kong Jr - Online");
    SetTargetFPS(60);
    
    printf("=== INICIANDO DEBUG ===\n");
    printf("Pantalla: %dx%d, Tile: %d\n", ANCHO_PANTALLA, ALTO_PANTALLA, TILE_SIZE);
    
    // ===== CONEXIÓN AL SERVIDOR =====
    printf("=== Conectando al servidor ===\n");
    bool conectado = false;
    if (conectar_servidor("localhost", 5000) == 0) {
        conectado = true;
        enviar_mensaje("CONNECT|PLAYER|JugadorC");
        printf("Mensaje CONNECT enviado\n");
    } else {
        printf("AVISO: No se pudo conectar al servidor - Modo offline\n");
        printf("Asegurate de que el servidor Java esté corriendo\n");
    }
    
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
    
    // --- VARIABLES DEL JUGADOR ---
    Vector2 cuadradoPos = {50, 100};
    int cuadradoSize = 50;
    float velocidad = 5.0f;
    bool enSuelo = false;
    bool enAgua = false;
    bool enLiana = false;
    bool sujetandoLiana = false;
    float velocidadY = 0.0f;
    const float gravedad = 0.5f;
    const float fuerzaSalto = -12.0f;
    
    // Variables para comunicación
    char buffer_recepcion[4096];
    bool movimientoEnviado = false;
    
    printf("=== INICIANDO BUCLE ===\n");
    
    // Bucle principal
    while (!WindowShouldClose()) {
        // ===== RECIBIR MENSAJES DEL SERVIDOR =====
        if (conectado && esta_conectado()) {
            int bytes = recibir_mensaje(buffer_recepcion, sizeof(buffer_recepcion));
            if (bytes > 0) {
                printf("[Servidor]: %s\n", buffer_recepcion);
                // TODO: Parsear GAMESTATE y actualizar posiciones de otros jugadores
            }
        }
        
        // --- MOVIMIENTO HORIZONTAL CON FLECHAS ---
        movimientoEnviado = false;
        if (IsKeyDown(KEY_RIGHT)) {
            cuadradoPos.x += velocidad;
            movimientoEnviado = true;
        }
        if (IsKeyDown(KEY_LEFT)) {
            cuadradoPos.x -= velocidad;
            movimientoEnviado = true;
        }
        
        // --- DETECCIÓN DE ESTADOS ---
        enSuelo = HayTileDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enAgua = HayAguaDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enLiana = HayLiana(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        
        // --- CONTROL MANUAL DE LIANAS (Tecla Z para agarrar/soltar) ---
        if (enLiana && IsKeyPressed(KEY_Z)) {
            sujetandoLiana = !sujetandoLiana;
            if (conectado && esta_conectado()) {
                enviar_mensaje(sujetandoLiana ? "ACTION|1|GRAB_LIANA" : "ACTION|1|RELEASE_LIANA");
            }
        }
        
        // --- SALTO CON ESPACIO ---
        if ((enSuelo || sujetandoLiana) && IsKeyPressed(KEY_SPACE)) {
            velocidadY = fuerzaSalto;
            enSuelo = false;
            sujetandoLiana = false;
            
            if (conectado && esta_conectado()) {
                enviar_mensaje("ACTION|1|JUMP");
            }
        }
        
        // --- COMPORTAMIENTO EN LIANA (solo si está sujetando) ---
        if (sujetandoLiana && enLiana) {
            // Permitir subir y bajar en la liana
            if (IsKeyDown(KEY_UP)) {
                cuadradoPos.y -= velocidad;
                movimientoEnviado = true;
            }
            else if (IsKeyDown(KEY_DOWN)) {
                cuadradoPos.y += velocidad;
                movimientoEnviado = true;
            }
            
            // Movimiento horizontal reducido en liana
            if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad * 0.3f;
            if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad * 0.3f;
            
            // Anular gravedad mientras esté sujetando
            velocidadY = 0;
        }
        else {
            // --- COMPORTAMIENTO NORMAL (no sujetando liana) ---
            sujetandoLiana = false;
            
            // Aplicar gravedad
            velocidadY += gravedad;
            cuadradoPos.y += velocidadY;
        }
        
        // --- ENVÍO DE POSICIÓN AL SERVIDOR ---
        if (conectado && esta_conectado() && movimientoEnviado) {
            char posMsg[100];
            snprintf(posMsg, sizeof(posMsg), "POS|1|%.0f|%.0f", cuadradoPos.x, cuadradoPos.y);
            enviar_mensaje(posMsg);
        }
        
        // --- COMPORTAMIENTO EN AGUA ---
        if (enAgua) {
            cuadradoPos.x = 50;
            cuadradoPos.y = 600;
            velocidadY = 0;
            sujetandoLiana = false;
            
            if (conectado && esta_conectado()) {
                enviar_mensaje("ACTION|1|WATER_RESPAWN");
            }
        }
        
        // --- CORRECCIÓN DE COLISIÓN CON SUELO ---
        if (enSuelo && velocidadY > 0) {
            int tileY = (int)((cuadradoPos.y + cuadradoSize) / mapa->tileSize);
            cuadradoPos.y = tileY * mapa->tileSize - cuadradoSize;
            velocidadY = 0;
            enSuelo = true;
            sujetandoLiana = false;
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
            if (conectado && esta_conectado()) colorCuadrado = GREEN;
            if (enLiana && !sujetandoLiana) colorCuadrado = ORANGE;
            if (sujetandoLiana) colorCuadrado = YELLOW;
            if (enAgua) colorCuadrado = SKYBLUE;
            
            DrawRectangle(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, colorCuadrado);
            DrawRectangleLines(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, WHITE);
            
            // UI de debug y estado
            DrawText("DON CEY KONG JR - ONLINE", 10, 10, 20, YELLOW);
            
            // Estado de conexión
            if (conectado && esta_conectado()) {
                DrawText("CONECTADO AL SERVIDOR", 10, 35, 15, GREEN);
            } else {
                DrawText("MODO OFFLINE", 10, 35, 15, RED);
            }
            
            // Información del juego
            DrawText(TextFormat("Posicion: (%.0f, %.0f)", cuadradoPos.x, cuadradoPos.y), 10, 55, 15, GREEN);
            DrawText(TextFormat("En suelo: %s", enSuelo ? "SI" : "NO"), 10, 75, 15, enSuelo ? GREEN : RED);
            DrawText(TextFormat("Liana disponible: %s", enLiana ? "SI" : "NO"), 10, 95, 15, enLiana ? ORANGE : RED);
            DrawText(TextFormat("Sujetando liana: %s", sujetandoLiana ? "SI" : "NO"), 10, 115, 15, sujetandoLiana ? GREEN : RED);
            DrawText(TextFormat("En agua: %s", enAgua ? "SI" : "NO"), 10, 135, 15, enAgua ? BLUE : RED);
            
            // Controles
            DrawText("Flechas: mover | ESPACIO: saltar | Z: agarrar/soltar liana", 10, 155, 12, RED);
            DrawText("Presiona ESC para salir", 10, ALTO_PANTALLA - 25, 15, WHITE);
            
        EndDrawing();
    }
    
    // Limpieza
    if (conectado) {
        desconectar_servidor();
    }
    LiberarMapa(mapa);
    CloseWindow();
    
    printf("=== JUEGO TERMINADO ===\n");
    return 0;
}