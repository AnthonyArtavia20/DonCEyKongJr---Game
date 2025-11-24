#ifndef ENEMIGOS_H
#define ENEMIGOS_H

#include "C:/msys64/mingw64/include/raylib.h"
#include "mapa.h"

#define MAX_ENEMIGOS 10
#define MAX_LIANAS 20
#define ALTO_PANTALLA 800

typedef enum {
    COCODRILO_AZUL = 1,  // <--- Cambiado para coincidir con Java (1 = Azul)
    COCODRILO_ROJO = 2   // <--- Cambiado para coincidir con Java (2 = Rojo)
} TipoEnemigo;

typedef struct {
    int id;
    TipoEnemigo tipo;
    Vector2 posicion;
    Vector2 velocidad;
    int activo;
    int enLiana;
    int lianaActual;
    float tiempoCambioDireccion;
    int direccion;
    Rectangle hitbox;
    float tiempoEspera;
} Enemigo;

typedef struct {
    int id;
    int tileX;
    int tileY_inicio;
    int tileY_fin;
    float x_pos;
} LianaInfo;

typedef struct {
    Enemigo enemigos[MAX_ENEMIGOS];
    LianaInfo lianas[MAX_LIANAS];
    int cantidad_enemigos;
    int cantidad_lianas;
    Mapa* mapa;
    int proximo_id;  // <--- NUEVO: Para IDs automáticos
} GestorEnemigos;

// Funciones principales
void InicializarEnemigos(GestorEnemigos* gestor, Mapa* mapa);
int CrearEnemigo(GestorEnemigos* gestor, int id, TipoEnemigo tipo, float x, float y);
int CrearEnemigoEnLiana(GestorEnemigos* gestor, int id, TipoEnemigo tipo, int lianaID);

// NUEVA FUNCIÓN PARA JAVA
int CrearEnemigoDesdeJava(GestorEnemigos* gestor, int tipoEnemigo, int lianaID);  // <--- NUEVA

void EliminarEnemigo(GestorEnemigos* gestor, int id);
void ActualizarEnemigos(GestorEnemigos* gestor, float deltaTime);
void DibujarEnemigos(GestorEnemigos* gestor);
Enemigo* BuscarEnemigo(GestorEnemigos* gestor, int id);
int BuscarIndiceEnemigo(GestorEnemigos* gestor, int id);

// Sistema de lianas
void IdentificarLianasEnMapa(GestorEnemigos* gestor);
void DebugLianas(GestorEnemigos* gestor);
LianaInfo* ObtenerLianaPorID(GestorEnemigos* gestor, int lianaID);
float ObtenerPosicionXLianaPorID(GestorEnemigos* gestor, int lianaID);

// Funciones específicas por tipo
void ActualizarCocodriloAzul(Enemigo* enemigo, GestorEnemigos* gestor, float deltaTime);
void ActualizarCocodriloRojo(Enemigo* enemigo, GestorEnemigos* gestor, float deltaTime);

// Utilidades
int HayLianaEnPosicion(GestorEnemigos* gestor, float x, float y);
int BuscarLianaCercana(GestorEnemigos* gestor, float x, float y, float rango);
int BuscarLianaCercanaID(GestorEnemigos* gestor, float x, float y, float rango);
float ObtenerPosicionXLiana(GestorEnemigos* gestor, int lianaIndex);

#endif