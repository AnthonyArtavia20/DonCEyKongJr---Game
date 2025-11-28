#include "C:/msys64/mingw64/include/raylib.h"
#include "mapa.h"
#include "socket_client.h"
#include "enemigos.h"
#include <stdio.h>
#include <string.h>
#include "fruta.h"


#define ANCHO_PANTALLA 1200
#define ALTO_PANTALLA 800
#define TILE_SIZE 17
#define VIDA_MAXIMA 3
#define VELOCIDAD_BASE_ENEMIGOS 1.0f

// Declarar global (correcto)
GestorEnemigos gestorEnemigos;
GestorFrutas gestorFrutas;


// [AGREGAR funciones nuevas]
int VerificarColisionConEnemigos(GestorEnemigos* gestor, float x, float y, int ancho, int alto) {
    Rectangle jugadorRect = {x, y, ancho, alto};
    
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo) {
            if (CheckCollisionRecs(jugadorRect, gestor->enemigos[i].hitbox)) {
                return gestor->enemigos[i].id;
            }
        }
    }
    return -1; // No hay colisión
}

int VerificarMeta(Mapa *mapa, float x, float y, int ancho, int alto) {
    if (!mapa) return 0;
    
    // Verificar varios puntos del jugador
    float puntosX[] = {x, x + ancho * 0.5f, x + ancho - 1};
    float puntosY[] = {y, y + alto * 0.5f, y + alto - 1};
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int tileX = (int)(puntosX[i] / mapa->tileSize);
            int tileY = (int)(puntosY[j] / mapa->tileSize);
            
            if (tileX >= 0 && tileX < mapa->ancho && tileY >= 0 && tileY < mapa->alto) {
                if (GetTile(mapa, tileX, tileY) == tile_meta) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

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
    
    // ===== SISTEMA DE NIVELES Y VIDAS =====
    int nivel = 1;
    int salud = VIDA_MAXIMA;
    bool juegoActivo = true;
    bool gameOver = false;
    int mostrarMensajeNivel = 0;
    float tiempoMensaje = 0.0f;
    
    // ===== INICIALIZAR SISTEMA DE ENEMIGOS =====
    InicializarEnemigos(&gestorEnemigos, mapa);
    InicializarFrutas(&gestorFrutas);


    /////////////////////////////////////DEBUG////////////////////////////////////////////

    // ===== CREAR ENEMIGOS DE PRUEBA (TEMPORAL) =====
    printf("=== Creando enemigos de prueba ===\n");
    CrearEnemigoEnLiana(&gestorEnemigos, 1, COCODRILO_AZUL, 1); // Liana ID 1
    CrearEnemigoEnLiana(&gestorEnemigos, 2, COCODRILO_AZUL, 2); // Liana ID 2  
    CrearEnemigoEnLiana(&gestorEnemigos, 3, COCODRILO_AZUL, 3); // Liana ID 3
    CrearEnemigoEnLiana(&gestorEnemigos, 4, COCODRILO_AZUL, 4); // Liana ID 1
    CrearEnemigoEnLiana(&gestorEnemigos, 5, COCODRILO_AZUL, 5); // Liana ID 2  
    CrearEnemigoEnLiana(&gestorEnemigos, 6, COCODRILO_AZUL, 6); // Liana ID 3
    CrearEnemigoEnLiana(&gestorEnemigos, 7, COCODRILO_ROJO, 7); // Liana ID 1
    CrearEnemigoEnLiana(&gestorEnemigos, 8, COCODRILO_AZUL, 8); // Liana ID 2  
    CrearEnemigoEnLiana(&gestorEnemigos, 9, COCODRILO_AZUL, 11); // Liana ID 3
    CrearEnemigoEnLiana(&gestorEnemigos, 10, COCODRILO_ROJO, 12); // Liana ID 1

    /////////////////////////////////////DEBUG////////////////////////////////////////////

    
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

        if (strncmp(buffer_recepcion, "FRUIT_CREATED", 13) == 0) {
            int vine, height, points;

            if (sscanf(buffer_recepcion, "FRUIT_CREATED|%d|%d|%d",
                    &vine, &height, &points) == 3) {

                CrearFruta(&gestorFrutas, vine, height, points);

            } else {
                printf("[Servidor] FRUIT_CREATED formato inválido\n");
            }
        }

        if (strncmp(buffer_recepcion, "CCA_CREATED", 11) == 0) {
            int vine, height, points;
            int nuevoID = gestorEnemigos.proximo_id;

            if (sscanf(buffer_recepcion, "CCA_CREATED|%d|%d|%d",
                    &vine, &height, &points) == 3) {

                printf("[Servidor] CCA_CREATED formato inválido----------------\n");

                CrearEnemigoEnLiana(&gestorEnemigos, nuevoID, COCODRILO_AZUL, vine);
                gestorEnemigos.proximo_id++; // Incrementar para el siguiente enemigo

            } else {
                printf("[Servidor] CCA_CREATED formato inválido\n");
            }
        }

    }
}
        
        // ===== ACTUALIZAR ENEMIGOS (FUERA DEL BLOQUE DE RECEPCIÓN) =====
        ActualizarEnemigos(&gestorEnemigos, GetFrameTime());
        
        // ===== SISTEMA DE COLISIONES Y VIDAS =====
        if (juegoActivo && !gameOver) {
            // Verificar colisión con enemigos
            int enemigoColisionado = VerificarColisionConEnemigos(&gestorEnemigos, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
            if (enemigoColisionado != -1) {
                salud--;
                printf("[Juego] ¡Colisión con enemigo! Salud: %d/%d\n", salud, VIDA_MAXIMA);

                if (esta_conectado()) {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "ENEMY_HIT|1|%d|%d", enemigoColisionado, salud);
                    enviar_mensaje(msg);
                    printf("[Socket] Enviado al servidor: %s\n", msg);
                }

                // Reposicionar jugador
                cuadradoPos = (Vector2){50, 100};
                velocidadY = 0;
                sujetandoLiana = false;
                
                if (salud <= 0) {
                    gameOver = true;
                    printf("[Juego] ¡GAME OVER!\n");
                }
            }
            
            // Verificar si llegó a la meta
            if (VerificarMeta(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize)) {
                nivel++;
                salud = VIDA_MAXIMA; // Recuperar toda la vida al pasar nivel
                CambiarNivelEnemigos(&gestorEnemigos, nivel);
                mostrarMensajeNivel = nivel - 1;
                tiempoMensaje = 0.0f;
                
                printf("[Juego] ¡NIVEL %d COMPLETADO! Pasando al nivel %d\n", nivel-1, nivel);
                
                // Reposicionar jugador para nuevo nivel
                cuadradoPos = (Vector2){50, 100};
                velocidadY = 0;
                sujetandoLiana = false;
                
                if (conectado && esta_conectado()) {
                    char mensaje[50];
                    snprintf(mensaje, sizeof(mensaje), "ACTION|1|LEVEL_UP|%d", nivel);
                    enviar_mensaje(mensaje);
                }
            }
        }

        // ===== REINICIAR JUEGO CON R =====
        if (gameOver && IsKeyPressed(KEY_R)) {
            nivel = 1;
            salud = VIDA_MAXIMA;
            gameOver = false;
            cuadradoPos = (Vector2){50, 100};
            velocidadY = 0;
            sujetandoLiana = false;
            CambiarNivelEnemigos(&gestorEnemigos, nivel);
            
            // Eliminar todos los enemigos existentes
            for (int i = 0; i < MAX_ENEMIGOS; i++) {
                if (gestorEnemigos.enemigos[i].activo) {
                    gestorEnemigos.enemigos[i].activo = 0;
                }
            }
            gestorEnemigos.cantidad_enemigos = 0;
            
            printf("[Juego] Juego reiniciado\n");
        }
        
        // Actualizar tiempo del mensaje de nivel
        if (mostrarMensajeNivel > 0) {
            tiempoMensaje += GetFrameTime();
            if (tiempoMensaje > 3.0f) {
                mostrarMensajeNivel = 0;
                tiempoMensaje = 0.0f;
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
            
            // Dibujar enemigos
            DibujarEnemigos(&gestorEnemigos);

            DibujarFrutas(&gestorFrutas);

            // Debug de enemigos
    DrawText(TextFormat("Enemigos: %d/%d", gestorEnemigos.cantidad_enemigos, MAX_ENEMIGOS), 10, 200, 15, PURPLE);

    // Dibujar hitboxes de enemigos (debug visual)
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestorEnemigos.enemigos[i].activo) {
            DrawRectangleLinesEx(gestorEnemigos.enemigos[i].hitbox, 2, RED);
            DrawText(TextFormat("E%d", gestorEnemigos.enemigos[i].id), 
                gestorEnemigos.enemigos[i].posicion.x, 
                gestorEnemigos.enemigos[i].posicion.y - 20, 10, WHITE);
    }
}

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
            
            // Información de nivel y salud
            DrawText(TextFormat("Nivel: %d", nivel), 10, 155, 15, YELLOW);
            DrawText(TextFormat("Salud: %d/%d", salud, VIDA_MAXIMA), 10, 175, 15, 
                    (salud == 3) ? GREEN : (salud == 2) ? YELLOW : RED);
            
            // Información de enemigos
            DrawText(TextFormat("Enemigos activos: %d", gestorEnemigos.cantidad_enemigos), 10, 195, 15, PURPLE);
            
            // Mensaje de nivel completado
            if (mostrarMensajeNivel > 0) {
                DrawText(TextFormat("¡NIVEL %d COMPLETADO!", mostrarMensajeNivel), 
                        ANCHO_PANTALLA/2 - 150, 50, 30, GREEN);
            }
            
            // Mensaje de Game Over
            if (gameOver) {
                DrawRectangle(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, (Color){0, 0, 0, 200});
                DrawText("¡GAME OVER!", ANCHO_PANTALLA/2 - 150, ALTO_PANTALLA/2 - 50, 40, RED);
                DrawText(TextFormat("Alcanzaste el nivel %d", nivel), ANCHO_PANTALLA/2 - 120, ALTO_PANTALLA/2, 20, WHITE);
                DrawText("Presiona R para reiniciar", ANCHO_PANTALLA/2 - 100, ALTO_PANTALLA/2 + 50, 20, GREEN);
            }
            
            // Controles
            DrawText("Flechas: mover | ESPACIO: saltar | Z: agarrar/soltar liana", 10, 215, 12, RED);
            DrawText("Presiona ESC para salir", 10, ALTO_PANTALLA - 25, 15, WHITE);
            
        EndDrawing();
    }
    
    // Limpieza
    if (conectado) {
        desconectar_servidor();
    }
    LiberarTexturasEnemigos(&gestorEnemigos);
    LiberarMapa(mapa);
    CloseWindow();
    
    printf("=== JUEGO TERMINADO ===\n");
    return 0;
}