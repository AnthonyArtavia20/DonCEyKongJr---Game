#include "enemigos.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Velocidades de los enemigos
#define VELOCIDAD_BASE 1.0f
#define VELOCIDAD_COCODRILO_AZUL 2.0f
#define VELOCIDAD_COCODRILO_ROJO 1.5f
#define RANGO_BUSQUEDA_LIANA 100.0f

float CalcularVelocidadSegunNivel(float velocidadBase, int nivel) {
    return velocidadBase * (1.0f + (nivel * 0.25f));
}

void InicializarEnemigos(GestorEnemigos* gestor, Mapa* mapa) {
    gestor->cantidad_enemigos = 0;
    gestor->cantidad_lianas = 0;
    gestor->mapa = mapa;
    gestor->proximo_id = 1;
    gestor->nivel_actual = 1;
    
    // Inicializar texturas
    gestor->tex_cocodrilo_azul = (Texture2D){0};
    gestor->tex_cocodrilo_rojo = (Texture2D){0};
    
    // Cargar texturas
    CargarTexturasEnemigos(gestor);
    
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
    
    printf("[Enemigos] Gestor inicializado. Enemigos: %d, Lianas: %d, Proximo ID: %d, Nivel: %d\n", 
           gestor->cantidad_enemigos, gestor->cantidad_lianas, gestor->proximo_id, gestor->nivel_actual);
}

// ===== SISTEMA DE TEXTURAS =====

void CargarTexturasEnemigos(GestorEnemigos* gestor) {
    printf("[Enemigos] Cargando texturas de enemigos...\n");
    
    // Cargar textura del cocodrilo azul
    if (FileExists("assets/cocodrilo_azul.png")) {
        Image img_azul = LoadImage("assets/cocodrilo_azul.png");
        gestor->tex_cocodrilo_azul = LoadTextureFromImage(img_azul);
        UnloadImage(img_azul);
        printf("[Enemigos] Textura cocodrilo_azul.png cargada: %dx%d\n", 
               gestor->tex_cocodrilo_azul.width, gestor->tex_cocodrilo_azul.height);
    } else {
        printf("[Enemigos] AVISO: assets/cocodrilo_azul.png no encontrado\n");
    }
    
    // Cargar textura del cocodrilo rojo
    if (FileExists("assets/cocodrilo_rojo.png")) {
        Image img_rojo = LoadImage("assets/cocodrilo_rojo.png");
        gestor->tex_cocodrilo_rojo = LoadTextureFromImage(img_rojo);
        UnloadImage(img_rojo);
        printf("[Enemigos] Textura cocodrilo_rojo.png cargada: %dx%d\n", 
               gestor->tex_cocodrilo_rojo.width, gestor->tex_cocodrilo_rojo.height);
    } else {
        printf("[Enemigos] AVISO: assets/cocodrilo_rojo.png no encontrado\n");
    }
}

void LiberarTexturasEnemigos(GestorEnemigos* gestor) {
    if (gestor->tex_cocodrilo_azul.id != 0) {
        UnloadTexture(gestor->tex_cocodrilo_azul);
        printf("[Enemigos] Textura cocodrilo_azul liberada\n");
    }
    
    if (gestor->tex_cocodrilo_rojo.id != 0) {
        UnloadTexture(gestor->tex_cocodrilo_rojo);
        printf("[Enemigos] Textura cocodrilo_rojo liberada\n");
    }
}

// ===== FUNCIÓN DIBUJAR ENEMIGOS ACTUALIZADA =====

void DibujarEnemigos(GestorEnemigos* gestor) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!gestor->enemigos[i].activo) continue;
        
        Texture2D* textura = NULL;
        Color colorDebug;
        const char* tipoTexto;
        
        switch (gestor->enemigos[i].tipo) {
            case COCODRILO_AZUL:
                if (gestor->tex_cocodrilo_azul.id != 0) {
                    textura = &gestor->tex_cocodrilo_azul;
                }
                colorDebug = BLUE;
                tipoTexto = "AZUL";
                break;
            case COCODRILO_ROJO:
                if (gestor->tex_cocodrilo_rojo.id != 0) {
                    textura = &gestor->tex_cocodrilo_rojo;
                }
                colorDebug = RED;
                tipoTexto = "ROJO";
                break;
            default:
                colorDebug = GRAY;
                tipoTexto = "DESCONOCIDO";
        }
        
        // Dibujar sprite si la textura está cargada
        if (textura && textura->id != 0) {
            // TAMAÑO DESEADO PARA EL SPRITE (puedes ajustar estos valores)
            float anchoDeseado = 60.0f;   // Sprite de 60px de ancho
            float altoDeseado = 60.0f;    // Sprite de 60px de alto
            
            // Calcular escalas
            float escalaX = anchoDeseado / textura->width;
            float escalaY = altoDeseado / textura->height;
            
            // Usar la escala más pequeña para mantener proporciones, o la más grande para llenar
            float escala = (escalaX < escalaY) ? escalaX : escalaY; // Mantiene proporción
            // float escala = (escalaX > escalaY) ? escalaX : escalaY; // Llena el espacio (puede distorsionar)
            
            // Calcular dimensiones escaladas
            float anchoEscalado = textura->width * escala;
            float altoEscalado = textura->height * escala;
            
            // Calcular posición centrada en el hitbox
            float offsetX = (gestor->enemigos[i].hitbox.width - anchoEscalado) / 2;
            float offsetY = (gestor->enemigos[i].hitbox.height - altoEscalado) / 2;
            
            Rectangle destRect = {
                gestor->enemigos[i].posicion.x + offsetX,
                gestor->enemigos[i].posicion.y + offsetY,
                anchoEscalado,
                altoEscalado
            };
            
            // Dibujar el sprite
            DrawTexturePro(*textura, 
                          (Rectangle){0, 0, (float)textura->width, (float)textura->height},
                          destRect,
                          (Vector2){0, 0},
                          0.0f,
                          WHITE);
        } else {
            // Si no hay textura, dibujar cuadro de debug
            DrawRectangleRec(gestor->enemigos[i].hitbox, colorDebug);
        }
        
        // Dibujar hitbox para colisiones (semi-transparente para debug)
        DrawRectangleLinesEx(gestor->enemigos[i].hitbox, 2, (Color){colorDebug.r, colorDebug.g, colorDebug.b, 150});
        
        // Indicador de tipo (solo en debug)
        #ifdef DEBUG
        DrawText(TextFormat("C%s", tipoTexto), 
                gestor->enemigos[i].posicion.x + 5, 
                gestor->enemigos[i].posicion.y - 15, 10, WHITE);
        
        if (gestor->enemigos[i].enLiana) {
            DrawText("LIANA", gestor->enemigos[i].posicion.x + 5, 
                    gestor->enemigos[i].posicion.y - 30, 8, GREEN);
        }
        #endif
    }
}

// ===== NUEVA FUNCIÓN PARA COMANDOS DE JAVA =====

int CrearEnemigoDesdeJava(GestorEnemigos* gestor, int tipoEnemigo, int lianaID) {
    if (!gestor->mapa) {
        printf("[Java] ERROR: No hay mapa referencia\n");
        return -1;
    }
    
    // Validar tipo de enemigo
    TipoEnemigo tipo;
    if (tipoEnemigo == 1) {
        tipo = COCODRILO_AZUL;
    } else if (tipoEnemigo == 2) {
        tipo = COCODRILO_ROJO;
    } else {
        printf("[Java] ERROR: Tipo de enemigo inválido: %d (1=Azul, 2=Rojo)\n", tipoEnemigo);
        return -1;
    }
    
    // Validar liana
    LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
    if (!liana) {
        printf("[Java] ERROR: Liana ID %d no encontrada\n", lianaID);
        DebugLianas(gestor);
        return -1;
    }
    
    // Verificar si hay espacio para más enemigos
    if (gestor->cantidad_enemigos >= MAX_ENEMIGOS) {
        printf("[Java] ERROR: Límite de enemigos alcanzado (%d/%d)\n", gestor->cantidad_enemigos, MAX_ENEMIGOS);
        return -1;
    }
    
    // Obtener ID automático
    int nuevoID = gestor->proximo_id;
    
    // Crear el enemigo
    float startY = liana->tileY_inicio * gestor->mapa->tileSize;
    printf("[Java] Creando enemigo ID %d, tipo %d en liana %d - Pos: (%.0f, %.0f)\n", 
           nuevoID, tipoEnemigo, lianaID, liana->x_pos, startY);
    
    if (CrearEnemigo(gestor, nuevoID, tipo, liana->x_pos - 15, startY)) {
        int idx = BuscarIndiceEnemigo(gestor, nuevoID);
        if (idx != -1) {
            gestor->enemigos[idx].enLiana = 1;
            gestor->enemigos[idx].lianaActual = lianaID;
            
            // Incrementar el próximo ID para el siguiente enemigo
            gestor->proximo_id++;
            
            printf("[Java] Enemigo creado exitosamente - ID: %d\n", nuevoID);
            return nuevoID;
        }
    }
    
    return -1;
}

// ===== FUNCIÓN ORIGINAL DE CREAR ENEMIGO =====

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
            gestor->enemigos[i].hitbox.width = 30;
            gestor->enemigos[i].hitbox.height = 30;
            
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
        DebugLianas(gestor);
        return 0;
    }
    
    float startY = liana->tileY_inicio * gestor->mapa->tileSize;
    
    printf("[Enemigos] Creando enemigo en liana %d - Pos: (%.0f, %.0f)\n", 
           lianaID, liana->x_pos, startY);
    
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
            gestor->cantidad_enemigos--;
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
    
    // CALCULAR VELOCIDAD ACTUAL SEGÚN NIVEL
    float velocidadActual = CalcularVelocidadSegunNivel(VELOCIDAD_COCODRILO_AZUL, gestor->nivel_actual);

    if (!enemigo->enLiana) {
        int lianaID = BuscarLianaCercanaID(gestor, enemigo->posicion.x, enemigo->posicion.y, RANGO_BUSQUEDA_LIANA);
        
        if (lianaID != -1) {
            LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
            if (liana) {
                enemigo->posicion.x = liana->x_pos - enemigo->hitbox.width/2 + 8;
                enemigo->posicion.y = liana->tileY_inicio * gestor->mapa->tileSize;
                enemigo->enLiana = 1;
                enemigo->lianaActual = lianaID;
                enemigo->velocidad = (Vector2){0, velocidadActual}; // USAR VELOCIDAD CALCULADA
                printf("[CocodriloAzul] ID %d se posicionó en liana %d - Velocidad: %.2f (Nivel %d)\n", 
                       enemigo->id, lianaID, velocidadActual, gestor->nivel_actual);
            }
        } else {
            enemigo->tiempoEspera += deltaTime;
            if (enemigo->tiempoEspera > 2.0f) {
                enemigo->posicion.x = (float)(rand() % (gestor->mapa->ancho * gestor->mapa->tileSize));
                enemigo->tiempoEspera = 0.0f;
            }
            enemigo->velocidad.x = 0;
            enemigo->velocidad.y = velocidadActual;  // USAR VELOCIDAD CALCULADA
        }
    } else {
        enemigo->velocidad.x = 0;
        enemigo->velocidad.y = velocidadActual;  // USAR VELOCIDAD CALCULADA
        
        LianaInfo* lianaActual = ObtenerLianaPorID(gestor, enemigo->lianaActual);
        int llegoAlFinal = 0;
        
        if (lianaActual) {
            float posYEnTiles = (enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize;
            if (posYEnTiles >= lianaActual->tileY_fin) {
                llegoAlFinal = 1;
            }
        }
        
        int tileX = (int)((enemigo->posicion.x + enemigo->hitbox.width/2) / gestor->mapa->tileSize);
        int tileY = (int)((enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize);
        
        int enLianaActual = 0;
        if (tileX >= 0 && tileX < gestor->mapa->ancho && tileY >= 0 && tileY < gestor->mapa->alto) {
            int tile = GetTile(gestor->mapa, tileX, tileY);
            enLianaActual = (tile == tile_liana);
        }
        
        if (llegoAlFinal || !enLianaActual || enemigo->posicion.y > ALTO_PANTALLA - 100) {
            printf("[CocodriloAzul] ID %d llegó al final de la liana - ELIMINANDO\n", enemigo->id);
            enemigo->activo = 0;
            gestor->cantidad_enemigos--;
            return;
        }
    }
    
    enemigo->posicion.x += enemigo->velocidad.x;
    enemigo->posicion.y += enemigo->velocidad.y;
    
    if (enemigo->posicion.y > ALTO_PANTALLA + 100) {
        enemigo->activo = 0;
        gestor->cantidad_enemigos--;
    }
}

// ===== COCODRILO ROJO =====
void ActualizarCocodriloRojo(Enemigo* enemigo, GestorEnemigos* gestor, float deltaTime) {
    if (!gestor->mapa) return;
    
    // CALCULAR VELOCIDAD ACTUAL SEGÚN NIVEL
    float velocidadActual = CalcularVelocidadSegunNivel(VELOCIDAD_COCODRILO_ROJO, gestor->nivel_actual);
    
    LianaInfo* lianaActual = ObtenerLianaPorID(gestor, enemigo->lianaActual);
    int enLianaActual = 0;
    
    if (lianaActual) {
        float posYEnTiles = enemigo->posicion.y / gestor->mapa->tileSize;
        float posYInferiorEnTiles = (enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize;
        
        if (posYInferiorEnTiles >= lianaActual->tileY_inicio && posYEnTiles <= lianaActual->tileY_fin) {
            enLianaActual = 1;
        }
    }
    
    if (!enLianaActual) {
        int lianaID = BuscarLianaCercanaID(gestor, enemigo->posicion.x, enemigo->posicion.y, RANGO_BUSQUEDA_LIANA);
        if (lianaID != -1) {
            LianaInfo* liana = ObtenerLianaPorID(gestor, lianaID);
            if (liana) {
                enemigo->posicion.x = liana->x_pos - enemigo->hitbox.width/2 + 8;
                float centroLiana = (liana->tileY_inicio + liana->tileY_fin) / 2.0f * gestor->mapa->tileSize;
                enemigo->posicion.y = centroLiana;
                enemigo->enLiana = 1;
                enemigo->lianaActual = lianaID;
                enemigo->direccion = -1;
            }
        } else {
            enemigo->velocidad = (Vector2){0, 0};
            return;
        }
    }
    
    if (lianaActual) {
        float minY = lianaActual->tileY_inicio * gestor->mapa->tileSize;
        float maxY = lianaActual->tileY_fin * gestor->mapa->tileSize - enemigo->hitbox.height;
        
        if (enemigo->posicion.y <= minY) {
            enemigo->posicion.y = minY;
            enemigo->direccion = 1;
        }
        else if (enemigo->posicion.y >= maxY) {
            enemigo->posicion.y = maxY;
            enemigo->direccion = -1;
        }
        
        int tileX = (int)((enemigo->posicion.x + enemigo->hitbox.width/2) / gestor->mapa->tileSize);
        int tileYSuperior = (int)(enemigo->posicion.y / gestor->mapa->tileSize);
        int tileYInferior = (int)((enemigo->posicion.y + enemigo->hitbox.height) / gestor->mapa->tileSize);
        
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
        
        if (enemigo->direccion < 0 && !hayLianaArriba) {
            enemigo->direccion = 1;
        }
        else if (enemigo->direccion > 0 && !hayLianaAbajo) {
            enemigo->direccion = -1;
        }
    }
    
    enemigo->velocidad.y = velocidadActual * enemigo->direccion;  // USAR VELOCIDAD CALCULADA
    enemigo->velocidad.x = 0;
    
    enemigo->posicion.y += enemigo->velocidad.y;
    
    if (enemigo->posicion.y > ALTO_PANTALLA + 100 || enemigo->posicion.y < -100) {
        enemigo->activo = 0;
        gestor->cantidad_enemigos--;
        printf("[CocodriloRojo] ID %d eliminado (fuera de pantalla)\n", enemigo->id);
    }
}

// ===== FUNCIONES DE UTILIDAD =====

int HayLianaEnPosicion(GestorEnemigos* gestor, float x, float y) {
    if (!gestor->mapa) return 0;
    
    int tileX = (int)(x / gestor->mapa->tileSize);
    int tileY = (int)(y / gestor->mapa->tileSize);
    
    if (tileX >= 0 && tileX < gestor->mapa->ancho && tileY >= 0 && tileY < gestor->mapa->alto) {
        return (GetTile(gestor->mapa, tileX, tileY) == tile_liana);
    }
    return 0;
}

int BuscarLianaCercanaID(GestorEnemigos* gestor, float x, float y, float rango) {
    if (!gestor->mapa) return -1;
    
    int mejorLianaID = -1;
    float mejorDistancia = rango * 2;
    
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

int BuscarLianaCercana(GestorEnemigos* gestor, float x, float y, float rango) {
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

void CambiarNivelEnemigos(GestorEnemigos* gestor, int nuevoNivel) {
    gestor->nivel_actual = nuevoNivel;
    printf("[Enemigos] Nivel cambiado a: %d\n", nuevoNivel);
    
    // Actualizar velocidades de todos los enemigos activos
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (gestor->enemigos[i].activo) {
            float velocidadActual;
            switch (gestor->enemigos[i].tipo) {
                case COCODRILO_AZUL:
                    velocidadActual = CalcularVelocidadSegunNivel(VELOCIDAD_COCODRILO_AZUL, nuevoNivel);
                    break;
                case COCODRILO_ROJO:
                    velocidadActual = CalcularVelocidadSegunNivel(VELOCIDAD_COCODRILO_ROJO, nuevoNivel);
                    break;
                default:
                    velocidadActual = 0;
            }
            
            // Aplicar la nueva velocidad manteniendo la dirección
            if (gestor->enemigos[i].tipo == COCODRILO_ROJO) {
                gestor->enemigos[i].velocidad.y = velocidadActual * gestor->enemigos[i].direccion;
            } else {
                gestor->enemigos[i].velocidad.y = velocidadActual;
            }
            
            printf("[Enemigos] Enemigo ID %d - Nueva velocidad: %.2f\n", 
                   gestor->enemigos[i].id, velocidadActual);
        }
    }
}

// ===== SISTEMA DE IDENTIFICACIÓN DE LIANAS =====

void IdentificarLianasEnMapa(GestorEnemigos* gestor) {
    if (!gestor->mapa) return;
    
    printf("=== IDENTIFICANDO LIANAS EN EL MAPA ===\n");
    
    int lianaID = 1;
    
    for (int x = 0; x < gestor->mapa->ancho; x++) {
        int tieneLianas = 0;
        int y_inicio = -1;
        int y_fin = -1;
        
        for (int y = 0; y < gestor->mapa->alto; y++) {
            if (GetTile(gestor->mapa, x, y) == tile_liana) {
                if (y_inicio == -1) {
                    y_inicio = y;
                }
                y_fin = y;
                tieneLianas = 1;
            } else if (y_inicio != -1) {
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