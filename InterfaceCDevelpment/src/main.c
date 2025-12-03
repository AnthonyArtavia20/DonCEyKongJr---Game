#include "C:/msys64/mingw64/include/raylib.h"
#include "mapa.h"
#include "socket_client.h"
#include "enemigos.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fruta.h"

#define ANCHO_PANTALLA 1200
#define ALTO_PANTALLA 800
#define TILE_SIZE 17
#define VIDA_MAXIMA 3
#define VELOCIDAD_BASE_ENEMIGOS 1.0f
#define MAX_REMOTE_PLAYERS 16

// Declarar global
GestorEnemigos gestorEnemigos;
GestorFrutas gestorFrutas;

// ✅ NUEVO: Variables para espectador
static int watchedPlayerId = 1;      // Por defecto observa al jugador 1
static bool showAllPlayers = false;  // false = solo uno, true = todos
static int targetRoomId = 1;         // Sala que estamos observando (para espectadores)

typedef struct {
    int id;
    int roomId;
    Vector2 pos;
    int activo;
    char nombre[32];
} RemotePlayer;

RemotePlayer remotePlayers[MAX_REMOTE_PLAYERS];

static void InicializarRemotePlayers(void) {
    for (int i = 0; i < MAX_REMOTE_PLAYERS; i++) {
        remotePlayers[i].id = -1;
        remotePlayers[i].pos = (Vector2){0,0};
        remotePlayers[i].activo = 0;
        remotePlayers[i].nombre[0] = '\0';
        remotePlayers[i].roomId = -1;
    }
}

static int FindRemoteIndexById(int id) {
    if (id <= 0) return -1;
    for (int i = 0; i < MAX_REMOTE_PLAYERS; i++) {
        if (remotePlayers[i].activo && remotePlayers[i].id == id) return i;
    }
    return -1;
}

static int AllocRemoteSlot(void) {
    for (int i = 0; i < MAX_REMOTE_PLAYERS; i++) {
        if (!remotePlayers[i].activo) return i;
    }
    return -1;
}

static bool EsParaMiSala(const char* mensaje, int miSalaId, int miJugadorId, bool esEspectador) {
    // Si el mensaje contiene información de sala, verificar
    char* ptr = strstr(mensaje, "|");
    if (!ptr) return true; // Si no tiene formato de sala, aceptar por compatibilidad
    
    // Para mensajes PLAYER_POS
    if (strncmp(mensaje, "PLAYER_POS|", 11) == 0) {
        int roomId, pid;
        if (sscanf(mensaje, "PLAYER_POS|%d|%d", &roomId, &pid) == 2) {
            return (roomId == miSalaId);
        }
    }
    
    // Para frutas y enemigos, todos los mensajes deberían ser filtrados por el servidor
    // pero por si acaso, agregar lógica aquí
    
    return true;
}

static void UpsertRemotePlayer(int id, int roomId, float x, float y, const char* name) {
    int idx = FindRemoteIndexById(id);
    if (idx >= 0) {
        remotePlayers[idx].pos.x = x;
        remotePlayers[idx].pos.y = y;
        remotePlayers[idx].roomId = roomId; 
        if (name && name[0] != '\0') strncpy(remotePlayers[idx].nombre, name, sizeof(remotePlayers[idx].nombre)-1);
        return;
    }
    int slot = AllocRemoteSlot();
    if (slot < 0) return;
    remotePlayers[slot].id = id;
    remotePlayers[slot].roomId = roomId; 
    remotePlayers[slot].pos.x = x;
    remotePlayers[slot].pos.y = y;
    remotePlayers[slot].activo = 1;
    if (name && name[0] != '\0') strncpy(remotePlayers[slot].nombre, name, sizeof(remotePlayers[slot].nombre)-1);
    else snprintf(remotePlayers[slot].nombre, sizeof(remotePlayers[slot].nombre), "P%d", id);
}

static void RemoveRemotePlayer(int id) {
    int idx = FindRemoteIndexById(id);
    if (idx >= 0) {
        remotePlayers[idx].activo = 0;
        remotePlayers[idx].id = -1;
        remotePlayers[idx].nombre[0] = '\0';
        remotePlayers[idx].roomId = -1;
    }
}

int VerificarColisionConEnemigos(GestorEnemigos* gestor, float x, float y, int ancho, int alto) {
    Rectangle jugadorRect = {x, y, ancho, alto};
    
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo) {
            if (CheckCollisionRecs(jugadorRect, gestor->enemigos[i].hitbox)) {
                return gestor->enemigos[i].id;
            }
        }
    }
    return -1;
}

int VerificarMeta(Mapa *mapa, float x, float y, int ancho, int alto) {
    if (!mapa) return 0;
    
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

int main(int argc, char** argv) {
    // ===== PARSEO DE ARGUMENTOS =====
    bool observerMode = false;
    const char* clientName = "JugadorC";
    const char* serverHost = "localhost";
    int serverPort = 5000;
    watchedPlayerId = 1; // Por defecto observa jugador 1
    targetRoomId = 1; // Por defecto observa sala 1

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--spectator") == 0 || strcmp(argv[i], "--observer") == 0) {
                observerMode = true;
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    clientName = argv[i+1];
                    i++;
                }
            } else if (strcmp(argv[i], "--player") == 0) {
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    clientName = argv[i+1];
                    i++;
                }
            } else if (strcmp(argv[i], "--host") == 0) {
                if (i + 1 < argc) {
                    serverHost = argv[i+1];
                    i++;
                }
            } else if (strcmp(argv[i], "--port") == 0) {
                if (i + 1 < argc) {
                    serverPort = atoi(argv[i+1]);
                    i++;
                }
            } else if (strcmp(argv[i], "--watch") == 0) {
                if (i + 1 < argc) {
                    watchedPlayerId = atoi(argv[i+1]);
                    if (watchedPlayerId < 1) watchedPlayerId = 1;
                    i++;
                }
            }
            else if (strcmp(argv[i], "--room") == 0) {
                if (i + 1 < argc) {
                    targetRoomId = atoi(argv[i+1]);
                    if (targetRoomId < 1) targetRoomId = 1;
                    i++;
                }
            }
        }
    }

    // Inicializar ventana
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Don CEy Kong Jr - Online");
    SetTargetFPS(60);
    
    printf("=== INICIANDO DEBUG ===\n");
    printf("Pantalla: %dx%d, Tile: %d\n", ANCHO_PANTALLA, ALTO_PANTALLA, TILE_SIZE);
    printf("Conectando a servidor %s:%d\n", serverHost, serverPort);
    printf("Modo: %s\n", observerMode ? "ESPECTADOR" : "JUGADOR");
    if (observerMode) {
        printf("Observando jugador: %d (Sala: %d)\n", watchedPlayerId, targetRoomId);
    }

    // Inicializar lista de remotos
    InicializarRemotePlayers();

    // ===== CONEXIÓN AL SERVIDOR =====
    printf("=== Conectando al servidor ===\n");
    bool conectado = false;
    int playerId = -1;
    int puntuacion = 0;
    
    if (conectar_servidor(serverHost, serverPort) == 0) {
        conectado = true;
        if (observerMode) {
            char buf[256];
            snprintf(buf, sizeof(buf), "CONNECT|SPECTATOR|%s|%d", clientName, targetRoomId);
            enviar_mensaje(buf);
            printf("Conectado como SPECTATOR: %s (Sala: %d)\n", clientName, targetRoomId);
        } else {
            char buf[256];
            snprintf(buf, sizeof(buf), "CONNECT|PLAYER|%s", clientName);
            enviar_mensaje(buf);
            printf("Mensaje CONNECT enviado (PLAYER): %s\n", clientName);
        }
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
    
    if (!FileExists("assets/Fondo.png")) {
        printf("AVISO: assets/Fondo.png no existe - usando fondo azul\n");
    }
    
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
    
    char buffer_recepcion[4096];
    bool movimientoEnviado = false;

    printf("=== INICIANDO BUCLE ===\n");
    
    // ===== BUCLE PRINCIPAL =====
    while (!WindowShouldClose()) {
        // ===== RECIBIR MENSAJES DEL SERVIDOR =====
        if (conectado && esta_conectado()) {
            int bytes = recibir_mensaje(buffer_recepcion, sizeof(buffer_recepcion)-1);
            if (bytes > 0) {
                if (bytes < (int)sizeof(buffer_recepcion)) buffer_recepcion[bytes] = '\0';
                else buffer_recepcion[sizeof(buffer_recepcion)-1] = '\0';

                printf("[Socket] Recibido: %s\n", buffer_recepcion);

                if (!EsParaMiSala(buffer_recepcion, targetRoomId, playerId, observerMode)) {
                printf("[DEBUG] Mensaje ignorado (no es para sala %d): %s\n", targetRoomId, buffer_recepcion);
                continue;
            }

                // OK con PLAYER_ID
                if (strncmp(buffer_recepcion, "OK|PLAYER_ID|", 13) == 0) {
                    int id = -1;
                    int roomId = -1;
                    if (sscanf(buffer_recepcion, "OK|PLAYER_ID|%d|ROOM_ID|%d", &id, &roomId) >= 1) {
                        playerId = id;
                        targetRoomId = roomId;
                        printf("[Servidor] Asignado PLAYER_ID = %d, ROOM_ID = %d\n", playerId, targetRoomId);
                        const char* scorePtr = strstr(buffer_recepcion, "SCORE|");
                        if (scorePtr) {
                            int score = 0;
                            sscanf(scorePtr, "SCORE|%d", &score);
                            puntuacion = score;
                        }
                    }
                }

                // PLAYER_POS|<roomId>|<playerId>|<x>|<y>
                if (strncmp(buffer_recepcion, "PLAYER_POS|", 11) == 0) {
                    int roomId, pid;
                    float px, py;
                    if (sscanf(buffer_recepcion, "PLAYER_POS|%d|%d|%f|%f", 
                               &roomId, &pid, &px, &py) == 4) {
                        // Solo procesar si es nuestro jugador o si somos espectador
                        // y estamos observando esta sala
                        if (!observerMode) {
                            // Jugador: actualizar posiciones remotas (solo si están en nuestra sala)
                            UpsertRemotePlayer(pid, roomId, px, py, NULL);
                        } else if (observerMode && targetRoomId == roomId) {
                            // Espectador: solo actualizar si es de nuestra sala
                            UpsertRemotePlayer(pid, roomId, px, py, NULL);
                        }
                    }
                }

                // PLAYER_JOINED|<playerId>|<name>
                if (strncmp(buffer_recepcion, "PLAYER_JOINED|", 14) == 0) {
                    int pid;
                    char namebuf[64];
                    if (sscanf(buffer_recepcion, "PLAYER_JOINED|%d|%63[^\n]", &pid, namebuf) == 2) {
                        if (pid != playerId || observerMode) {
                            // Asignar sala por defecto (se actualizará con PLAYER_POS)
                            UpsertRemotePlayer(pid, targetRoomId, 50.0f, 100.0f, namebuf);
                        }
                    }
                }

                // PLAYER_LEFT|<playerId>
                if (strncmp(buffer_recepcion, "PLAYER_LEFT|", 12) == 0) {
                    int pid;
                    if (sscanf(buffer_recepcion, "PLAYER_LEFT|%d", &pid) == 1) {
                        RemoveRemotePlayer(pid);
                    }
                }

                // FRUIT_CREATED|<id>|<vine>|<height>|<points>
                if (strncmp(buffer_recepcion, "FRUIT_CREATED|", 14) == 0) {
                    int fid, vine, height, points;
                    if (sscanf(buffer_recepcion, "FRUIT_CREATED|%d|%d|%d|%d",
                            &fid, &vine, &height, &points) == 4) {
                        CrearFruta(&gestorFrutas, fid, vine, (float)height, points);
                        printf("[Servidor] FRUIT_CREATED id=%d vine=%d y=%d pts=%d\n", fid, vine, height, points);
                    }
                }

                // FRUIT_DELETED|<id>|<playerId>|<points>
                if (strncmp(buffer_recepcion, "FRUIT_DELETED|", 14) == 0) {
                    int fid, pid, points;
                    if (sscanf(buffer_recepcion, "FRUIT_DELETED|%d|%d|%d", &fid, &pid, &points) >= 2) {
                        EliminarFrutaPorId(&gestorFrutas, fid);
                        printf("[Servidor] FRUIT_DELETED id=%d by player=%d points=%d\n", fid, pid, points);
                    }
                }

                // SCORE_UPDATE|<playerId>|<score>
                if (strncmp(buffer_recepcion, "SCORE_UPDATE|", 13) == 0) {
                    int pid, score;
                    if (sscanf(buffer_recepcion, "SCORE_UPDATE|%d|%d", &pid, &score) == 2) {
                        if (pid == playerId) {
                            puntuacion = score;
                        }
                        printf("[Servidor] SCORE_UPDATE player=%d score=%d\n", pid, score);
                    }
                }

                // CCA_CREATED (enemigos)
                if (strncmp(buffer_recepcion, "CCA_CREATED", 11) == 0) {
        int vine, height, points;
        int nuevoID = gestorEnemigos.proximo_id;
        if (sscanf(buffer_recepcion, "CCA_CREATED|%d|%d|%d",
                &vine, &height, &points) == 3) {
            printf("[CLIENT] Cocodrilo AZUL creado vine=%d (Sala: %d)\n", vine, targetRoomId);
            CrearEnemigoEnLiana(&gestorEnemigos, nuevoID, COCODRILO_AZUL, vine);
            gestorEnemigos.proximo_id++;
        }
    }

                // CCR_CREATED (enemigos)
                if (strncmp(buffer_recepcion, "CCR_CREATED", 11) == 0) {
        int vine, height, points;
        int nuevoID = gestorEnemigos.proximo_id;
        if (sscanf(buffer_recepcion, "CCR_CREATED|%d|%d|%d",
                &vine, &height, &points) == 3) {
            printf("[CLIENT] Cocodrilo ROJO creado vine=%d (Sala: %d)\n", vine, targetRoomId);
            CrearEnemigoEnLiana(&gestorEnemigos, nuevoID, COCODRILO_ROJO, vine);
            gestorEnemigos.proximo_id++;
                    }
                }
            }
        }
        
        // ===== ACTUALIZAR ENEMIGOS =====
        ActualizarEnemigos(&gestorEnemigos, GetFrameTime());
        
        // ===== CONTROLES DEL ESPECTADOR =====
        if (observerMode) {
            // Tecla TAB: Cambiar entre jugadores
            if (IsKeyPressed(KEY_TAB)) {
                int originalId = watchedPlayerId;
                int attempts = 0;
                do {
                    watchedPlayerId++;
                    if (watchedPlayerId > 2) watchedPlayerId = 1;
                    attempts++;
                    
                    int idx = FindRemoteIndexById(watchedPlayerId);
                    if (idx >= 0 && remotePlayers[idx].roomId == targetRoomId) {
                        printf("[Espectador] Cambiando a observar Jugador %d (Sala %d)\n", watchedPlayerId, targetRoomId);
                        break;
                    }
                    
                } while (attempts < 3);
                
                if (attempts >= 3) {
                    watchedPlayerId = originalId;
                    printf("[Espectador] No hay otros jugadores disponibles en Sala %d\n", targetRoomId);
                }
            }
            
            // Tecla 1: Observar jugador 1
            if (IsKeyPressed(KEY_ONE)) {
                int idx = FindRemoteIndexById(1);
                if (idx >= 0 && remotePlayers[idx].roomId == targetRoomId) {
                    watchedPlayerId = 1;
                    printf("[Espectador] Observando Jugador 1 (Sala %d)\n", targetRoomId);
                } else {
                    printf("[Espectador] Jugador 1 no disponible en Sala %d\n", targetRoomId);
                }
            }
            
            // Tecla 2: Observar jugador 2
            if (IsKeyPressed(KEY_TWO)) {
                int idx = FindRemoteIndexById(2);
                if (idx >= 0 && remotePlayers[idx].roomId == targetRoomId) {
                    watchedPlayerId = 2;
                    printf("[Espectador] Observando Jugador 2 (Sala %d)\n", targetRoomId);
                } else {
                    printf("[Espectador] Jugador 2 no disponible en Sala %d\n", targetRoomId);
                }
            }
            
            // Tecla A: Ver TODOS los jugadores (toggle)
            if (IsKeyPressed(KEY_A)) {
                showAllPlayers = !showAllPlayers;
                printf("[Espectador] Mostrar todos en Sala %d: %s\n", targetRoomId, showAllPlayers ? "SI" : "NO");
            }
        }
        
        // ===== SISTEMA DE COLISIONES Y VIDAS (solo para jugadores) =====
        if (!observerMode && juegoActivo && !gameOver) {
            Rectangle jugadorRect = { cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize };

            // Colisión con enemigos
            int enemigoColisionado = VerificarColisionConEnemigos(&gestorEnemigos, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
            if (enemigoColisionado != -1) {
                salud--;
                printf("[Juego] ¡Colisión con enemigo! Salud: %d/%d\n", salud, VIDA_MAXIMA);

                if (esta_conectado() && playerId > 0) {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "ENEMY_HIT|%d|%d|%d", playerId, enemigoColisionado, 1);
                    enviar_mensaje(msg);
                }

                cuadradoPos = (Vector2){50, 100};
                velocidadY = 0;
                sujetandoLiana = false;
                
                if (salud <= 0) {
                    gameOver = true;
                    printf("[Juego] ¡GAME OVER!\n");
                }
            }
            
            // Colisión con frutas
            for (int i = 0; i < MAX_FRUTAS; i++) {
                if (!gestorFrutas.frutas[i].activo) continue;
                
                if (CheckCollisionRecs(jugadorRect, gestorFrutas.frutas[i].hitbox)) {
                    int fid = gestorFrutas.frutas[i].id;
                    int vine = gestorFrutas.frutas[i].liana;
                    int height = (int)gestorFrutas.frutas[i].posicion.y;
                    int points = gestorFrutas.frutas[i].puntos;
                    
                    if (esta_conectado() && fid >= 0 && playerId > 0) {
                        char hitMsg[128];
                        snprintf(hitMsg, sizeof(hitMsg), "HIT|%d|%d", fid, playerId);
                        enviar_mensaje(hitMsg);
                        printf("[Socket] Enviado HIT: %s\n", hitMsg);
                    }
                    
                    puntuacion += points;
                    EliminarFruta(&gestorFrutas, i);
                }
            }
            
            // Verificar meta
            if (VerificarMeta(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize)) {
                nivel++;
                salud = VIDA_MAXIMA;
                CambiarNivelEnemigos(&gestorEnemigos, nivel);
                mostrarMensajeNivel = nivel - 1;
                tiempoMensaje = 0.0f;
                
                printf("[Juego] ¡NIVEL %d COMPLETADO! Pasando al nivel %d\n", nivel-1, nivel);
                
                cuadradoPos = (Vector2){50, 100};
                velocidadY = 0;
                sujetandoLiana = false;
                
                if (conectado && esta_conectado() && playerId > 0) {
                    char mensaje[50];
                    snprintf(mensaje, sizeof(mensaje), "ACTION|%d|LEVEL_UP|%d", playerId, nivel);
                    enviar_mensaje(mensaje);
                }
            }
        }

        // ===== REINICIAR JUEGO CON R =====
        if (!observerMode && gameOver && IsKeyPressed(KEY_R)) {
            nivel = 1;
            salud = VIDA_MAXIMA;
            gameOver = false;
            cuadradoPos = (Vector2){50, 100};
            velocidadY = 0;
            sujetandoLiana = false;
            CambiarNivelEnemigos(&gestorEnemigos, nivel);
            
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
        
        // ===== MOVIMIENTO (solo jugadores) =====
        movimientoEnviado = false;
        if (!observerMode) {
            if (IsKeyDown(KEY_RIGHT)) {
                cuadradoPos.x += velocidad;
                movimientoEnviado = true;
            }
            if (IsKeyDown(KEY_LEFT)) {
                cuadradoPos.x -= velocidad;
                movimientoEnviado = true;
            }
        }
        
        // Detección de estados
        enSuelo = HayTileDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enAgua = HayAguaDebajo(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        enLiana = HayLiana(mapa, cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize);
        
        // Control de lianas
        if (!observerMode && enLiana && IsKeyPressed(KEY_Z)) {
            sujetandoLiana = !sujetandoLiana;
            if (conectado && esta_conectado() && playerId > 0) {
                enviar_mensaje(sujetandoLiana ? "ACTION|1|GRAB_LIANA" : "ACTION|1|RELEASE_LIANA");
            }
        }
        
        // Salto
        if (!observerMode && (enSuelo || sujetandoLiana) && IsKeyPressed(KEY_SPACE)) {
            velocidadY = fuerzaSalto;
            enSuelo = false;
            sujetandoLiana = false;
            
            if (conectado && esta_conectado() && playerId > 0) {
                enviar_mensaje("ACTION|1|JUMP");
            }
        }
        
        // Comportamiento en liana
        if (!observerMode && sujetandoLiana && enLiana) {
            if (IsKeyDown(KEY_UP)) {
                cuadradoPos.y -= velocidad;
                movimientoEnviado = true;
            }
            else if (IsKeyDown(KEY_DOWN)) {
                cuadradoPos.y += velocidad;
                movimientoEnviado = true;
            }
            
            if (IsKeyDown(KEY_RIGHT)) cuadradoPos.x += velocidad * 0.3f;
            if (IsKeyDown(KEY_LEFT)) cuadradoPos.x -= velocidad * 0.3f;
            
            velocidadY = 0;
        }
        else if (!observerMode) {
            sujetandoLiana = false;
            velocidadY += gravedad;
            cuadradoPos.y += velocidadY;
        }
        
        // Envío de posición
        if (!observerMode && conectado && esta_conectado() && movimientoEnviado && playerId > 0) {
            char posMsg[100];
            snprintf(posMsg, sizeof(posMsg), "POS|%d|%.1f|%.1f", playerId, cuadradoPos.x, cuadradoPos.y);
            enviar_mensaje(posMsg);
        }
        
        // Comportamiento en agua
        if (!observerMode && enAgua) {
            cuadradoPos.x = 50;
            cuadradoPos.y = 600;
            velocidadY = 0;
            sujetandoLiana = false;
            
            if (conectado && esta_conectado() && playerId > 0) {
                enviar_mensaje("ACTION|1|WATER_RESPAWN");
            }
        }
        
        // Corrección de colisión con suelo
        if (!observerMode && enSuelo && velocidadY > 0) {
            int tileY = (int)((cuadradoPos.y + cuadradoSize) / mapa->tileSize);
            cuadradoPos.y = tileY * mapa->tileSize - cuadradoSize;
            velocidadY = 0;
            enSuelo = true;
            sujetandoLiana = false;
        }
        
        // Límites de pantalla
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
        
        // ========== DIBUJADO ==========
        BeginDrawing();
            ClearBackground(BLACK);
            
            DibujarMapa(mapa);
            DibujarEnemigos(&gestorEnemigos);
            DibujarFrutas(&gestorFrutas);

            // Debug de enemigos
            for (int i = 0; i < MAX_ENEMIGOS; i++) {
                if (gestorEnemigos.enemigos[i].activo) {
                    DrawRectangleLinesEx(gestorEnemigos.enemigos[i].hitbox, 2, RED);
                    DrawText(TextFormat("E%d", gestorEnemigos.enemigos[i].id), 
                        gestorEnemigos.enemigos[i].posicion.x, 
                        gestorEnemigos.enemigos[i].posicion.y - 20, 10, WHITE);
                }
            }

            // ========== DIBUJAR JUGADORES ==========
            if (observerMode) {
                // ✅ MODO ESPECTADOR
                
                if (showAllPlayers) {
                    // Mostrar TODOS los jugadores de NUESTRA SALA
                    for (int i = 0; i < MAX_REMOTE_PLAYERS; i++) {
                        if (!remotePlayers[i].activo) continue;
                        if (remotePlayers[i].roomId != targetRoomId) continue; // Solo de nuestra sala
                        
                        Color playerColor = (remotePlayers[i].id == 1) ? BLUE : 
                                           (remotePlayers[i].id == 2) ? RED : GREEN;
                        
                        DrawRectangle(remotePlayers[i].pos.x, remotePlayers[i].pos.y, 
                                     cuadradoSize, cuadradoSize, playerColor);
                        DrawRectangleLines(remotePlayers[i].pos.x, remotePlayers[i].pos.y, 
                                          cuadradoSize, cuadradoSize, WHITE);
                        
                        const char* label = remotePlayers[i].nombre[0] ? 
                                           remotePlayers[i].nombre :
                                           TextFormat("P%d", remotePlayers[i].id);
                        DrawText(label, remotePlayers[i].pos.x, remotePlayers[i].pos.y - 16, 
                                10, YELLOW);
                    }
                
                    DrawText(TextFormat("ESPECTADOR - SALA %d - MOSTRANDO TODOS", targetRoomId), 10, 35, 15, GREEN);
                
                } else {
                    // Mostrar SOLO el jugador seleccionado de NUESTRA SALA
                    int targetIdx = FindRemoteIndexById(watchedPlayerId);
                    
                    if (targetIdx >= 0 && remotePlayers[targetIdx].activo && 
                        remotePlayers[targetIdx].roomId == targetRoomId) {
                        
                        Color playerColor = (watchedPlayerId == 1) ? BLUE : 
                                           (watchedPlayerId == 2) ? RED : GREEN;
                        
                        float px = remotePlayers[targetIdx].pos.x;
                        float py = remotePlayers[targetIdx].pos.y;
                        
                        DrawRectangle(px, py, cuadradoSize, cuadradoSize, playerColor);
                        DrawRectangleLines(px, py, cuadradoSize, cuadradoSize, WHITE);
                        
                        const char* label = remotePlayers[targetIdx].nombre[0] ? 
                                           remotePlayers[targetIdx].nombre : 
                                           TextFormat("P%d", watchedPlayerId);
                        DrawText(label, px, py - 16, 10, YELLOW);
                        
                        DrawText(TextFormat("OBSERVANDO: Jugador %d (Sala %d)", 
                                watchedPlayerId, targetRoomId), 10, 35, 15, GREEN);
                    } else {
                        DrawText(TextFormat("Esperando Jugador %d en Sala %d...", 
                                watchedPlayerId, targetRoomId), 
                                ANCHO_PANTALLA/2 - 100, ALTO_PANTALLA/2, 20, YELLOW);
                        DrawText(TextFormat("MODO ESPECTADOR - SALA %d", targetRoomId), 
                                10, 35, 15, ORANGE);
                    }
                }
            
            } else {
                // ✅ MODO JUGADOR
                
                // Dibujar jugador LOCAL
                Color colorCuadrado = BLUE;
                if (conectado && esta_conectado()) colorCuadrado = GREEN;
                if (enLiana && !sujetandoLiana) colorCuadrado = ORANGE;
                if (sujetandoLiana) colorCuadrado = YELLOW;
                if (enAgua) colorCuadrado = SKYBLUE;
                
                DrawRectangle(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, colorCuadrado);
                DrawRectangleLines(cuadradoPos.x, cuadradoPos.y, cuadradoSize, cuadradoSize, WHITE);
                DrawText(TextFormat("P%d (TÚ) - Sala %d", playerId, targetRoomId), 
                        cuadradoPos.x, cuadradoPos.y - 16, 10, LIME);
                
                // Dibujar OTROS jugadores (remotos) de NUESTRA SALA
                for (int i = 0; i < MAX_REMOTE_PLAYERS; i++) {
                    if (!remotePlayers[i].activo) continue;
                    if (remotePlayers[i].id == playerId) continue;
                    if (remotePlayers[i].roomId != targetRoomId) continue; // Solo de nuestra sala
                    
                    Color remoteColor = (remotePlayers[i].id == 1) ? DARKBLUE : 
                                       (remotePlayers[i].id == 2) ? DARKPURPLE : DARKGREEN;
                    
                    DrawRectangle(remotePlayers[i].pos.x, remotePlayers[i].pos.y, 
                                 cuadradoSize, cuadradoSize, remoteColor);
                    DrawRectangleLines(remotePlayers[i].pos.x, remotePlayers[i].pos.y, 
                                      cuadradoSize, cuadradoSize, ORANGE);
                    
                    const char* label = remotePlayers[i].nombre[0] ? 
                                       remotePlayers[i].nombre : 
                                       TextFormat("P%d", remotePlayers[i].id);
                    DrawText(label, remotePlayers[i].pos.x, remotePlayers[i].pos.y - 16, 
                            10, ORANGE);
                }
            }
        
            // ========== UI ==========
            DrawText("DON CEY KONG JR - ONLINE", 10, 10, 20, YELLOW);
            
            if (conectado && esta_conectado()) {
                DrawText(observerMode ? "ESPECTADOR CONECTADO" : "CONECTADO AL SERVIDOR", 10, 35, 15, GREEN);
            } else {
                DrawText("MODO OFFLINE", 10, 35, 15, RED);
            }
            
            
            if (!observerMode) {
                DrawText(TextFormat("Posicion: (%.1f, %.1f)", cuadradoPos.x, cuadradoPos.y), 10, 55, 15, GREEN);
                DrawText(TextFormat("En suelo: %s", enSuelo ? "SI" : "NO"), 10, 75, 15, enSuelo ? GREEN : RED);
                DrawText(TextFormat("Liana disponible: %s", enLiana ? "SI" : "NO"), 10, 95, 15, enLiana ? ORANGE : RED);
                DrawText(TextFormat("Sujetando liana: %s", sujetandoLiana ? "SI" : "NO"), 10, 115, 15, sujetandoLiana ? GREEN : RED);
                DrawText(TextFormat("En agua: %s", enAgua ? "SI" : "NO"), 10, 135, 15, enAgua ? BLUE : RED);
                DrawText(TextFormat("Nivel: %d", nivel), 10, 155, 15, YELLOW);
                DrawText(TextFormat("Salud: %d/%d", salud, VIDA_MAXIMA), 10, 175, 15, 
                        (salud == 3) ? GREEN : (salud == 2) ? YELLOW : RED);
                DrawText(TextFormat("Puntuacion: %d", puntuacion), 10, 195, 18, GOLD);
            }
            
            DrawText(TextFormat("Enemigos: %d/%d", gestorEnemigos.cantidad_enemigos, MAX_ENEMIGOS), 10, 220, 15, PURPLE);
            
            if (mostrarMensajeNivel > 0) {
                DrawText(TextFormat("¡NIVEL %d COMPLETADO!", mostrarMensajeNivel), 
                        ANCHO_PANTALLA/2 - 150, 50, 30, GREEN);
            }
            
            if (gameOver) {
                DrawRectangle(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, (Color){0, 0, 0, 200});
                DrawText("¡GAME OVER!", ANCHO_PANTALLA/2 - 150, ALTO_PANTALLA/2 - 50, 40, RED);
                DrawText(TextFormat("Alcanzaste el nivel %d", nivel), ANCHO_PANTALLA/2 - 120, ALTO_PANTALLA/2, 20, WHITE);
                DrawText("Presiona R para reiniciar", ANCHO_PANTALLA/2 - 100, ALTO_PANTALLA/2 + 50, 20, GREEN);
            }
            
            // Controles
            if (observerMode) {
                DrawText("TAB: cambiar jugador | 1/2: seleccionar | A: ver todos | ESC: salir", 
                        10, ALTO_PANTALLA - 25, 12, YELLOW);
            } else {
                DrawText("Flechas: mover | ESPACIO: saltar | Z: liana | ESC: salir", 
                        10, ALTO_PANTALLA - 25, 12, RED);
            }
        
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