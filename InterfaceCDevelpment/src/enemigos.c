#include "enemigos.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>  // <--- AÑADIR PARA TextFormat

// Velocidades de los enemigos
#define VELOCIDAD_COCODRILO_AZUL 2.0f
#define VELOCIDAD_COCODRILO_ROJO 1.5f
#define TIEMPO_CAMBIO_DIRECCION 3.0f
#define RANGO_BUSQUEDA_LIANA 100.0f

void InicializarEnemigos(GestorEnemigos* gestor, Mapa* mapa) {
    gestor->cantidad_enemigos = 0;
    gestor->cantidad_lianas = 0;
    gestor->mapa = mapa;
    
    // Inicializar enemigos
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        gestor->enemigos[i].activo = 0;
        gestor->enemigos[i].id = -1;
        gestor->enemigos[i].tiempoEspera = 0.0f;
    }
    
    // Inicializar lianas
    for (int i = 0; i < MAX_LIANAS; i++) {
        gestor->lianas[i].id = -1;
    }
    
    // Identificar lianas en el mapa
    IdentificarLianasEnMapa(gestor);
    
    printf("[Enemigos] Gestor inicializado. Enemigos: %d, Lianas identificadas: %d\n", 
           gestor->cantidad_enemigos, gestor->cantidad_lianas);
}

// ===== SISTEMA DE IDENTIFICACIÓN DE LIANAS =====

void IdentificarLianasEnMapa(GestorEnemigos* gestor) {
    if (!gestor->mapa) return;
    
    printf("=== IDENTIFICANDO LIANAS EN EL MAPA ===\n");
    
    int lianaID = 1;
    
    // Buscar columnas que contengan lianas
    for (int x = 0; x < gestor->mapa->ancho; x++) {
        int tieneLianas = 0;
        int y_inicio = -1;
        int y_fin = -1;
        
        // Buscar segmentos verticales de lianas en esta columna
        for (int y = 0; y < gestor->mapa->alto; y++) {
            if (GetTile(gestor->mapa, x, y) == tile_liana) {
                if (y_inicio == -1) {
                    y_inicio = y; // Inicio del segmento
                }
                y_fin = y; // Actualizar fin del segmento
                tieneLianas = 1;
            } else if (y_inicio != -1) {
                // Fin del segmento continuo, registrar liana
                if (gestor->cantidad_lianas < MAX_LIANAS) {
                    gestor->lianas[gestor->cantidad_lianas].id = lianaID;
                    gestor->lianas[gestor->cantidad_lianas].tileX = x;
                    gestor->lianas[gestor->cantidad_lianas].tileY_inicio = y_inicio;
                    gestor->lianas[gestor->cantidad_lianas].tileY_fin = y_fin;
                    gestor->lianas[gestor->cantidad_lianas].x_pos = x * gestor->mapa->tileSize;
                    
                    printf("Liana %d: Tile(%d,%d)-(%d,%d) -> PosX: %.0f\n", 
                           lianaID, x, y_inicio, x, y_fin, 
                           gestor->lianas[gestor->cantidad_lianas].x_pos);
                    
                    gestor->cantidad_lianas++;
                    lianaID++;
                }
                y_inicio = -1;
                y_fin = -1;
            }
        }
        
        // Registrar último segmento si existe
        if (y_inicio != -1 && gestor->cantidad_lianas < MAX_LIANAS) {
            gestor->lianas[gestor->cantidad_lianas].id = lianaID;
            gestor->lianas[gestor->cantidad_lianas].tileX = x;
            gestor->lianas[gestor->cantidad_lianas].tileY_inicio = y_inicio;
            gestor->lianas[gestor->cantidad_lianas].tileY_fin = y_fin;
            gestor->lianas[gestor->cantidad_lianas].x_pos = x * gestor->mapa->tileSize;
            
            printf("Liana %d: Tile(%d,%d)-(%d,%d) -> PosX: %.0f\n", 
                   lianaID, x, y_inicio, x, y_fin, 
                   gestor->lianas[gestor->cantidad_lianas].x_pos);
            
            gestor->cantidad_lianas++;
            lianaID++;
        }
    }
    
    printf("=== TOTAL LIANAS IDENTIFICADAS: %d ===\n", gestor->cantidad_lianas);
}

void DebugLianas(GestorEnemigos* gestor) {
    printf("=== DEBUG LIANAS ===\n");
    for (int i = 0; i < gestor->cantidad_lianas; i++) {
        printf("Liana %d: X=%d, Y=%d-%d, PosX=%.0f\n",
               gestor->lianas[i].id,
               gestor->lianas[i].tileX,
               gestor->lianas[i].tileY_inicio,
               gestor->lianas[i].tileY_fin,
               gestor->lianas[i].x_pos);
    }
}

LianaInfo* ObtenerLianaPorID(GestorEnemigos* gestor, int lianaID) {
    for (int i = 0; i < gestor->cantidad_lianas; i++) {
        if (gestor->lianas[i].id == lianaID) {
            return &gestor->lianas[i];
        }
    }
    return NULL;
}

float ObtenerPosicionXLianaPorID(GestorEnemigos* gestor, int lianaID) {
    LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
    if (liana) {
        return liana->x_pos;
    }
    return 0;
}

// ===== CREACIÓN DE ENEMIGOS =====

int CrearEnemigo(GestorEnemigos* gestor, int id, TipoEnemigo tipo, float x, float y) {
    if (gestor->cantidad_enemigos >= MAX_ENEMIGOS) {
        printf("[Enemigos] No se puede crear enemigo %d - Límite alcanzado\n", id);
        return 0;
    }
    
    // Buscar slot libre
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!gestor->enemigos[i].activo) {
            gestor->enemigos[i].id = id;
            gestor->enemigos[i].tipo = tipo;
            gestor->enemigos[i].posicion = (Vector2){x, y};
            gestor->enemigos[i].velocidad = (Vector2){0, 0};
            gestor->enemigos[i].activo = 1;
            gestor->enemigos[i].enLiana = 0;
            gestor->enemigos[i].lianaActual = -1;
            gestor->enemigos[i].tiempoCambioDireccion = 0.0f;
            gestor->enemigos[i].direccion = -1;
            gestor->enemigos[i].tiempoEspera = 0.0f;
            
            // Configurar hitbox
            gestor->enemigos[i].hitbox = (Rectangle){x, y, 30, 30};
            
            gestor->cantidad_enemigos++;
            printf("[Enemigos] Creado enemigo ID %d, tipo %d en (%.0f, %.0f)\n", id, tipo, x, y);
            return 1;
        }
    }
    
    return 0;
}

int CrearEnemigoEnLiana(GestorEnemigos* gestor, int id, TipoEnemigo tipo, int lianaID) {
    if (!gestor->mapa) {
        printf("[Enemigos] ERROR: No hay mapa referencia\n");
        return 0;
    }
    
    LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
    if (!liana) {
        printf("[Enemigos] ERROR: Liana ID %d no encontrada\n", lianaID);
        DebugLianas(gestor); // Mostrar lianas disponibles
        return 0;
    }
    
    // Calcular posición Y inicial (parte superior de la liana)
    float startY = liana->tileY_inicio * gestor->mapa->tileSize;
    
    printf("[Enemigos] Creando enemigo en liana %d - Pos: (%.0f, %.0f)\n", 
           lianaID, liana->x_pos, startY);
    
    // Crear el enemigo
    if (CrearEnemigo(gestor, id, tipo, liana->x_pos - 15, startY)) {
        int idx = BuscarIndiceEnemigo(gestor, id);
        if (idx != -1) {
            gestor->enemigos[idx].enLiana = 1;
            gestor->enemigos[idx].lianaActual = lianaID;
            printf("[Enemigos] Enemigo %d creado en liana %d\n", id, lianaID);
            return 1;
        }
    }
    
    return 0;
}

// Función auxiliar para buscar índice por ID
int BuscarIndiceEnemigo(GestorEnemigos* gestor, int id) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo && gestor->enemigos[i].id == id) {
            return i;
        }
    }
    return -1;
}

void EliminarEnemigo(GestorEnemigos* gestor, int id) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo && gestor->enemigos[i].id == id) {
            gestor->enemigos[i].activo = 0;
            gestor->cantidad_enemigos--;  // <--- CORREGIDO: cantidad_enemigos en lugar de cantidad
            printf("[Enemigos] Eliminado enemigo ID %d\n", id);
            return;
        }
    }
}

Enemigo* BuscarEnemigo(GestorEnemigos* gestor, int id) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo && gestor->enemigos[i].id == id) {
            return &gestor->enemigos[i];
        }
    }
    return NULL;
}

void ActualizarEnemigos(GestorEnemigos* gestor, float deltaTime) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!gestor->enemigos[i].activo) continue;
        
        switch (gestor->enemigos[i].tipo) {
            case COCODRILO_AZUL:
                ActualizarCocodriloAzul(&gestor->enemigos[i], gestor, deltaTime);
                break;
            case COCODRILO_ROJO:
                ActualizarCocodriloRojo(&gestor->enemigos[i], gestor, deltaTime);
                break;
        }
        
        // Actualizar hitbox
        gestor->enemigos[i].hitbox.x = gestor->enemigos[i].posicion.x;
        gestor->enemigos[i].hitbox.y = gestor->enemigos[i].posicion.y;
    }
}

// ===== COCODRILO AZUL =====
void ActualizarCocodriloAzul(Enemigo* enemigo, GestorEnemigos* gestor, float deltaTime) {
    if (!gestor->mapa) return;
    
    // Comportamiento del cocodrilo azul:
    // 1. Solo baja por la liana
    // 2. Cuando llega al final, se elimina
    
    if (!enemigo->enLiana) {
        // Buscar liana cercana usando el nuevo sistema
        int lianaID = BuscarLianaCercanaID(gestor, enemigo->posicion.x, enemigo->posicion.y, RANGO_BUSQUEDA_LIANA);
        
        if (lianaID != -1) {
            LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
            if (liana) {
                enemigo->posicion.x = liana->x_pos - enemigo->hitbox.width/2 + 8; // Centrar en liana
                enemigo->posicion.y = liana->tileY_inicio * gestor->mapa->tileSize; // Empezar desde arriba de la liana
                enemigo->enLiana = 1;
                enemigo->lianaActual = lianaID;
                enemigo->velocidad = (Vector2){0, VELOCIDAD_COCODRILO_AZUL};
                printf("[CocodriloAzul] ID %d se posicionó en liana %d\n", enemigo->id, lianaID);
            }
        } else {
            // Esperar hasta encontrar liana
            enemigo->tiempoEspera += deltaTime;
            if (enemigo->tiempoEspera > 2.0f) {
                // Reubicar aleatoriamente para buscar otra liana
                enemigo->posicion.x = (float)(rand() % (gestor->mapa->ancho * gestor->mapa->tileSize));
                enemigo->tiempoEspera = 0.0f;
            }
        }
    } else {
        // Ya está en liana - solo bajar
        enemigo->velocidad.x = 0;
        enemigo->velocidad.y = VELOCIDAD_COCODRILO_AZUL;
        
        // Verificar si llegó al final de la liana
        LianaInfo* lianaActual = ObtenerLianaPorID(gestor, enemigo->lianaActual);
        int llegoAlFinal = 0;
        
        if (lianaActual) {
            // Verificar si llegó al final de la liana (parte inferior)
            float posYEnTiles = (enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize;
            if (posYEnTiles >= lianaActual->tileY_fin) {
                llegoAlFinal = 1;
            }
        }
        
        // También verificar si se salió de la liana por colisión
        int tileX = (int)((enemigo->posicion.x + enemigo->hitbox.width/2) / gestor->mapa->tileSize);
        int tileY = (int)((enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize);
        
        int enLianaActual = 0;
        if (tileX >= 0 && tileX < gestor->mapa->ancho && tileY >= 0 && tileY < gestor->mapa->alto) {
            int tile = GetTile(gestor->mapa, tileX, tileY);
            enLianaActual = (tile == tile_liana);
        }
        
        if (llegoAlFinal || !enLianaActual || enemigo->posicion.y > ALTO_PANTALLA - 100) {
            // Llegó al final de la liana o se salió - ELIMINAR
            printf("[CocodriloAzul] ID %d llegó al final de la liana - ELIMINANDO\n", enemigo->id);
            enemigo->activo = 0;
            gestor->cantidad_enemigos--;
            return; // Salir de la función, el enemigo ya no existe
        }
    }
    
    // Aplicar movimiento
    enemigo->posicion.x += enemigo->velocidad.x;
    enemigo->posicion.y += enemigo->velocidad.y;
    
    // Si se sale completamente de pantalla, eliminar
    if (enemigo->posicion.y > ALTO_PANTALLA + 100) {
        enemigo->activo = 0;
        gestor->cantidad_enemigos--;
        printf("[CocodriloAzul] ID %d eliminado (fuera de pantalla)\n", enemigo->id);
    }
}

// ===== COCODRILO ROJO =====
void ActualizarCocodriloRojo(Enemigo* enemigo, GestorEnemigos* gestor, float deltaTime) {
    if (!gestor->mapa) return;
    
    // Comportamiento del cocodrilo rojo:
    // 1. Se mueve verticalmente en lianas
    // 2. Cambia dirección cuando detecta el final de la liana
    // 3. Nunca se elimina (a menos que salga de pantalla)
    
    // Verificar si está en una liana usando el nuevo sistema
    LianaInfo* lianaActual = ObtenerLianaPorID(gestor, enemigo->lianaActual);
    int enLianaActual = 0;
    
    if (lianaActual) {
        // Verificar si está dentro del rango Y de la liana
        float posYEnTiles = enemigo->posicion.y / gestor->mapa->tileSize;
        float posYInferiorEnTiles = (enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize;
        
        if (posYInferiorEnTiles >= lianaActual->tileY_inicio && posYEnTiles <= lianaActual->tileY_fin) {
            enLianaActual = 1;
        }
    }
    
    if (!enLianaActual) {
        // Perdió la liana, buscar una nueva usando el nuevo sistema
        int lianaID = BuscarLianaCercanaID(gestor, enemigo->posicion.x, enemigo->posicion.y, RANGO_BUSQUEDA_LIANA);
        if (lianaID != -1) {
            LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
            if (liana) {
                enemigo->posicion.x = liana->x_pos - enemigo->hitbox.width/2 + 8;
                // Posicionar en el centro de la liana
                float centroLiana = (liana->tileY_inicio + liana->tileY_fin) / 2.0f * gestor->mapa->tileSize;
                enemigo->posicion.y = centroLiana;
                enemigo->enLiana = 1;
                enemigo->lianaActual = lianaID;
                enemigo->direccion = -1; // Empezar bajando
                printf("[CocodriloRojo] ID %d se reubicó en liana %d\n", enemigo->id, lianaID);
            }
        } else {
            // No hay lianas cercanas, desactivar temporalmente
            enemigo->velocidad = (Vector2){0, 0};
            return;
        }
    }
    
    // Comportamiento normal en liana - DETECCIÓN DE FINALES
    if (lianaActual) {
        float minY = lianaActual->tileY_inicio * gestor->mapa->tileSize;
        float maxY = lianaActual->tileY_fin * gestor->mapa->tileSize - enemigo->hitbox.height;
        
        // Verificar si llegó al tope superior
        if (enemigo->posicion.y <= minY) {
            enemigo->posicion.y = minY;
            enemigo->direccion = 1; // Cambiar a bajar
            printf("[CocodriloRojo] ID %d llegó al tope - cambiando a BAJAR\n", enemigo->id);
        }
        // Verificar si llegó al tope inferior
        else if (enemigo->posicion.y >= maxY) {
            enemigo->posicion.y = maxY;
            enemigo->direccion = -1; // Cambiar a subir
            printf("[CocodriloRojo] ID %d llegó al fondo - cambiando a SUBIR\n", enemigo->id);
        }
        
        // También verificar por colisión con tiles
        int tileX = (int)((enemigo->posicion.x + enemigo->hitbox.width/2) / gestor->mapa->tileSize);
        int tileYSuperior = (int)(enemigo->posicion.y / gestor->mapa->tileSize);
        int tileYInferior = (int)((enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize);
        
        // Verificar si hay liana en la posición actual
        int hayLianaArriba = 0;
        int hayLianaAbajo = 0;
        
        if (tileX >= 0 && tileX < gestor->mapa->ancho) {
            if (tileYSuperior >= 0 && tileYSuperior < gestor->mapa->alto) {
                hayLianaArriba = (GetTile(gestor->mapa, tileX, tileYSuperior) == tile_liana);
            }
            if (tileYInferior >= 0 && tileYInferior < gestor->mapa->alto) {
                hayLianaAbajo = (GetTile(gestor->mapa, tileX, tileYInferior) == tile_liana);
            }
        }
        
        // Cambiar dirección si no hay liana en la dirección actual
        if (enemigo->direccion < 0 && !hayLianaArriba) { // Subiendo pero no hay liana arriba
            enemigo->direccion = 1; // Cambiar a bajar
            printf("[CocodriloRojo] ID %d - no hay liana arriba, cambiando a BAJAR\n", enemigo->id);
        }
        else if (enemigo->direccion > 0 && !hayLianaAbajo) { // Bajando pero no hay liana abajo
            enemigo->direccion = -1; // Cambiar a subir
            printf("[CocodriloRojo] ID %d - no hay liana abajo, cambiando a SUBIR\n", enemigo->id);
        }
    }
    
    // Movimiento vertical en la liana
    enemigo->velocidad.y = VELOCIDAD_COCODRILO_ROJO * enemigo->direccion;
    enemigo->velocidad.x = 0;
    
    // Aplicar movimiento
    enemigo->posicion.y += enemigo->velocidad.y;
    
    // Si se sale completamente de pantalla, eliminar (solo por seguridad)
    if (enemigo->posicion.y > ALTO_PANTALLA + 100 || enemigo->posicion.y < -100) {
        enemigo->activo = 0;
        gestor->cantidad_enemigos--;
        printf("[CocodriloRojo] ID %d eliminado (fuera de pantalla)\n", enemigo->id);
    }
}

// ===== FUNCIONES DE UTILIDAD MEJORADAS =====

int HayLianaEnPosicion(GestorEnemigos* gestor, float x, float y) {
    if (!gestor->mapa) return 0;
    
    int tileX = (int)(x / gestor->mapa->tileSize);
    int tileY = (int)(y / gestor->mapa->tileSize);
    
    if (tileX >= 0 && tileX < gestor->mapa->ancho && tileY >= 0 && tileY < gestor->mapa->alto) {
        return (GetTile(gestor->mapa, tileX, tileY) == tile_liana);
    }
    return 0;
}

// Nueva función que usa el sistema de IDs
int BuscarLianaCercanaID(GestorEnemigos* gestor, float x, float y, float rango) {
    if (!gestor->mapa) return -1;
    
    int mejorLianaID = -1;
    float mejorDistancia = rango * 2; // Inicializar con valor grande
    
    for (int i = 0; i < gestor->cantidad_lianas; i++) {
        float distX = fabs(gestor->lianas[i].x_pos - x);
        float distY = fabs((gestor->lianas[i].tileY_inicio * gestor->mapa->tileSize) - y);
        
        if (distX <= rango && distY <= rango) {
            float distanciaTotal = sqrtf(distX * distX + distY * distY);
            if (distanciaTotal < mejorDistancia) {
                mejorDistancia = distanciaTotal;
                mejorLianaID = gestor->lianas[i].id;
            }
        }
    }
    
    return mejorLianaID;
}

// Función original mantenida por compatibilidad
int BuscarLianaCercana(GestorEnemigos* gestor, float x, float y, float rango) {
    // Usar la nueva función y convertir resultado
    int lianaID = BuscarLianaCercanaID(gestor, x, y, rango);
    if (lianaID != -1) {
        LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
        return liana ? liana->tileX : -1;
    }
    return -1;
}

float ObtenerPosicionXLiana(GestorEnemigos* gestor, int lianaIndex) {
    if (!gestor->mapa) return 0;
    return lianaIndex * gestor->mapa->tileSize;
}

void DibujarEnemigos(GestorEnemigos* gestor) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!gestor->enemigos[i].activo) continue;
        
        Color color;
        const char* tipoTexto;
        
        switch (gestor->enemigos[i].tipo) {
            case COCODRILO_AZUL:
                color = BLUE;
                tipoTexto = "AZUL";
                break;
            case COCODRILO_ROJO:
                color = RED;
                tipoTexto = "ROJO";
                break;
            default:
                color = GRAY;
                tipoTexto = "DESCONOCIDO";
        }
        
        // Dibujar enemigo
        DrawRectangleRec(gestor->enemigos[i].hitbox, color);
        DrawRectangleLinesEx(gestor->enemigos[i].hitbox, 2, DARKGRAY);
        
        // Indicador de tipo
        DrawText(TextFormat("C%s", tipoTexto), 
                gestor->enemigos[i].posicion.x + 5, 
                gestor->enemigos[i].posicion.y - 15, 10, WHITE);
        
        // Indicador de estado (debug)
        if (gestor->enemigos[i].enLiana) {
            DrawText("LIANA", gestor->enemigos[i].posicion.x + 5, 
                    gestor->enemigos[i].posicion.y - 30, 8, GREEN);
        }
    }
}