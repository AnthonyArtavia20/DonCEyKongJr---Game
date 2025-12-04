# Don CEy Kong Jr - Game Online 🎮

Juego multijugador en línea inspirado en Donkey Kong Jr. con servidor Java y cliente C/Raylib.

## 📋 Requisitos

- **Java JDK 11+** (para compilar y ejecutar el servidor)
- **MSYS2 con GCC** (para compilar el cliente C)
- **Raylib** (incluido en MSYS2)

### ⚠️ Instalación de MSYS2 (Windows)

Si no tienes MSYS2 instalado:
1. Descarga desde: https://www.msys2.org/
2. Instala en la ruta por defecto: `C:\msys64`
3. Agrega al PATH de Windows: `C:\msys64\mingw64\bin`

## 🚀 Inicio Rápido

### Paso 1: Setup (Una sola vez)
```bash
cd "Proyecto 4 desarrollo/DonCEyKongJr---Game"
bash setup.sh
```

Este script:
- ✅ Configura el PATH con MSYS2
- ✅ Compila el Servidor Java
- ✅ Compila el Cliente C
- ✅ Copia DLLs necesarias de Raylib

### Paso 2: Ejecutar Servidor (Terminal 1)
```bash
bash run_server.sh
```

Verás:
```
=================================
  DonCEy Kong Jr Server
  Puerto: 5000
  Max Jugadores: 2
  Max Espectadores: 4
  Nivel inicial: 1
=================================

Comandos disponibles:
  stats  - Mostrar estadísticas del servidor
  rooms  - Mostrar salas activas
  cf     - Crear Fruta en sala específica
  cca    - Crear Cocodrilo Azul en sala específica
  ccr    - Crear Cocodrilo Rojo en sala específica
  quit   - Detener servidor
```

### Paso 3: Ejecutar Cliente (Terminal 2)
```bash
bash run_client.sh
```

Se abrirá la ventana del juego y listo, **¡a jugar!** 🎮

---

## 📁 Estructura del Proyecto

```
DonCEyKongJr---Game/
├── setup.sh                           # ⭐ Script de configuración inicial
├── run_server.sh                      # ⭐ Ejecutar servidor
├── run_client.sh                      # ⭐ Ejecutar cliente
├── README.md                          # 📖 Este archivo
│
├── GameServer/                        # 🎮 Servidor del juego (Java)
│   ├── bin/                           # Clases compiladas (.class)
│   │   └── GameServer/
│   │       ├── CoreGenericServer/     # Framework genérico del servidor
│   │       └── DonkeyKong/
│   │           ├── Game/              # Lógica del juego compilada
│   │           │   ├── factory/       # Patrón Factory (niveles)
│   │           │   ├── model/         # Modelos (Enemy, Fruit, etc)
│   │           │   └── Observer/      # Patrón Observer (eventos)
│   │           └── Server/            # Manejadores de clientes
│   │
│   ├── CoreGenericServer/             # 📦 Código fuente del framework
│   │   ├── ClientHandler.java        # Manejador base de clientes
│   │   ├── GameServer.java           # Clase base del servidor
│   │   ├── MessageProtocol.java      # Protocolo de mensajes
│   │   └── ServerConfig.java         # Configuración del servidor
│   │
│   └── DonkeyKong/                    # 🐒 Código fuente específico del juego
│       ├── Client/                    # (Reservado para cliente Java futuro)
│       │   └── assets/                # Recursos gráficos
│       │
│       ├── Game/                      # 🎯 Lógica del juego
│       │   ├── factory/               # Patrón Factory para niveles
│       │   │   ├── EnemyFactory.java          # Interfaz Factory
│       │   │   ├── Level1EnemyFactory.java    # Factory Nivel 1
│       │   │   ├── Level2EnemyFactory.java    # Factory Nivel 2
│       │   │   └── Level3EnemyFactory.java    # Factory Nivel 3
│       │   │
│       │   ├── model/                 # Modelos de entidades
│       │   │   ├── Entity.java                # Clase base
│       │   │   ├── Enemy.java                 # Enemigo base
│       │   │   ├── BlueCrocodile.java         # Cocodrilo azul
│       │   │   ├── RedCrocodile.java          # Cocodrilo rojo
│       │   │   ├── Fruit.java                 # Fruta coleccionable
│       │   │   └── Collectible.java           # Objetos coleccionables
│       │   │
│       │   ├── Observer/              # Patrón Observer para eventos
│       │   │   ├── GameObserver.java          # Interfaz Observer
│       │   │   ├── GameEvent.java             # Eventos del juego
│       │   │   └── BroadcastManager.java      # Subject (notificador)
│       │   │
│       │   └── GameLogic.java         # 🧠 Lógica principal del juego
│       │
│       └── Server/                    # 🌐 Servidor específico
│           ├── DKClientHandler.java   # Manejador de clientes DK
│           └── DonkeyKongServer.java  # Servidor principal (main)
│
└── InterfaceCDevelpment/              # 🎨 Cliente del juego (C/Raylib)
    ├── .vscode/                       # Configuración de VSCode
    │   └── settings.json
    │
    ├── include/                       # 📄 Headers (.h)
    │   ├── mapa.h                     # Renderizado del mapa
    │   ├── Socket_client.h            # Cliente de red
    │   └── raylib.h                   # Raylib (si no está en system)
    │
    ├── obj/                           # Objetos compilados (.o)
    │   ├── main.o
    │   ├── mapa.o
    │   └── Socket_client.o
    │
    ├── src/                           # 💻 Código fuente (.c)
    │   ├── main.c                     # Punto de entrada del cliente
    │   ├── mapa.c                     # Lógica de renderizado
    │   └── Socket_client.c            # Comunicación con servidor
    │
    ├── Makefile                       # ⚙️ Configuración de compilación
    ├── client.exe                     # Ejecutable compilado
    └── *.dll                          # DLLs de Raylib (copiadas por setup.sh)
```

---

## 🔌 Protocolo de Comunicación

### Mensajes Cliente → Servidor

| Tipo            | Formato                                    | Ejemplo                                    |
|-----------------|--------------------------------------------|--------------------------------------------|
| **Conexión**    | `CONNECT\|<tipo>\|<nombre>[\|<room_id>]`   | `CONNECT\|PLAYER\|Juan`                    |
| **Espectador**  | `CONNECT\|SPECTATOR\|<nombre>\|<room_id>`  | `CONNECT\|SPECTATOR\|Pedro\|1`             |
| **Admin**       | `CONNECT\|ADMIN\|<nombre>`                 | `CONNECT\|ADMIN\|Server_Admin`             |
| **Posición**    | `POS\|<player_id>\|<x>\|<y>`               | `POS\|1\|325.5\|450.0`                     |
| **Golpe fruta** | `HIT\|<fruit_id>\|<player_id>`             | `HIT\|5\|1`                                |
| **Enemigo hit** | `ENEMY_HIT\|<player_id>\|<enemy_id>\|<dmg>`| `ENEMY_HIT\|1\|3\|1`                      |
| **Acción**      | `ACTION\|<player_id>\|<action>\|<param>`   | `ACTION\|1\|LEVEL_UP\|2`                   |

### Mensajes Servidor → Cliente

| Tipo               | Formato                                    | Descripción                    |
|--------------------|--------------------------------------------|--------------------------------|
| **OK**             | `OK\|PLAYER_ID\|<id>\|ROOM_ID\|<room>...`  | Confirmación de conexión       |
| **ERROR**          | `ERROR\|<code>\|<message>`                 | Notificación de error          |
| **PLAYER_POS**     | `PLAYER_POS\|<room>\|<pid>\|<x>\|<y>`      | Posición de jugador            |
| **FRUIT_CREATED**  | `FRUIT_CREATED\|<id>\|<vine>\|<h>\|<pts>\|<room>` | Fruta creada            |
| **FRUIT_DELETED**  | `FRUIT_DELETED\|<id>\|<pid>\|<points>`     | Fruta eliminada                |
| **CCA_CREATED**    | `CCA_CREATED\|<vine>\|0\|0\|<room>`        | Cocodrilo azul creado          |
| **CCR_CREATED**    | `CCR_CREATED\|<vine>\|0\|0\|<room>`        | Cocodrilo rojo creado          |
| **SCORE_UPDATE**   | `SCORE_UPDATE\|<player_id>\|<score>`       | Actualización de puntaje       |
| **PLAYER_JOINED**  | `PLAYER_JOINED\|<player_id>\|<name>`       | Jugador se unió                |
| **PLAYER_LEFT**    | `PLAYER_LEFT\|<player_id>`                 | Jugador se desconectó          |

---

## 🎮 Comandos del Servidor (CLI)

Mientras el servidor está ejecutándose, puedes usar estos comandos:

| Comando                  | Descripción                                  |
|--------------------------|----------------------------------------------|
| `stats`                  | Muestra estadísticas (jugadores, salas, etc) |
| `rooms`                  | Lista todas las salas activas                |
| `cf`                     | Crear fruta (modo interactivo)               |
| `cca`                    | Crear cocodrilo azul (modo interactivo)      |
| `ccr`                    | Crear cocodrilo rojo (modo interactivo)      |
| `df`                     | Eliminar fruta por ID (modo interactivo)     |
| `deletef <sala> <id>`    | Eliminar fruta por ID (inline)               |
| `level <sala> <nivel>`   | Cambiar nivel de una sala (1-3)              |
| `debug`                  | Mostrar información de debug                 |
| `quit` / `exit` / `stop` | Detener el servidor                          |

### Ejemplo de uso:
```bash
# Ver salas activas
> rooms

╔════════════════════════════════════════════════════════════╗
║                     SALAS ACTIVAS                           ║
╠════╦═══════════╦═══════╦═════════╦═══════════╦═════════════╣
║ Sala║ Jugador   ║ Nivel ║ Enemigos║ Frutas    ║ Espectadores║
╠════╬═══════════╬═══════╬═════════╬═══════════╬═════════════╣
║  1 ║ Juan      ║  2    ║  3      ║  5        ║  1          ║
╚════╩═══════════╩═══════╩═════════╩═══════════╩═════════════╝

# Crear fruta en sala 1
> cf
Ingrese número de sala (1-1): 1
Ingrese la liana para la fruta (1-9): 5
Ingrese altura en la liana (100-700): 400
Ingrese puntos de la fruta (50-500): 150

[SERVER] ✓ Fruta creada (id=7) en Sala 1, liana 5, altura 400, puntos 150
```
---

## 🎯 Controles del Cliente

| Tecla       | Acción                        |
|-------------|-------------------------------|
| **← →**     | Mover izquierda/derecha       |
| **↑ ↓**     | Subir/bajar en liana          |
| **ESPACIO** | Saltar                        |
| **Z**       | Agarrar/soltar liana          |
| **ESC**     | Salir del juego               |
---
## 🛠️ Comandos Útiles

### Recompilar todo desde cero
```bash
bash setup.sh
```
### Limpiar archivos compilados
```bash
# Limpiar cliente
cd InterfaceCDevelpment
make clean

# Limpiar servidor
cd ../GameServer
rm -rf bin/*
```

---

## 🧪 Testing

### Probar conexión múltiple (2 jugadores + 2 espectadores)

**Terminal 1** (Servidor):
```bash
bash run_server.sh
```

**Terminal 2** (Jugador 1):
```bash
bash run_client.sh
# Ingresa nombre: Juan
```

**Terminal 3** (Jugador 2):
```bash
bash run_client.sh
# Ingresa nombre: Pedro
```

**Terminal 4** (Espectador en sala 1):
```bash
bash run_client.sh
# Ingresa tipo: SPECTATOR
# Ingresa sala: 1
```

---
