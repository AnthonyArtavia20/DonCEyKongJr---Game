# Don CEy Kong Jr - Game Online 🎮

Juego multijugador en línea inspirado en Donkey Kong Jr. con servidor Java y cliente C/Raylib.

## 📋 Requisitos

- **Java JDK** (para compilar y ejecutar el servidor)
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
╔════════════════════════════════════════════════════════════╗
║   Iniciando Servidor Java - Don CEy Kong Jr              ║
╚════════════════════════════════════════════════════════════╝

Servidor escuchando en puerto: 5000
Máximo de jugadores: 4
```

### Paso 3: Ejecutar Cliente (Terminal 2)

```bash
bash run_client.sh
```

Se abrirá la ventana del juego con:
- Control: Flechas para mover, ESPACIO para saltar, Z para agarrar lianas
- Conexión automática al servidor en `localhost:5000`
- enderizado en tiempo real con Raylib

## 📁 Estructura del Proyecto

```
DonCEyKongJr---Game/
├── setup.sh                    # ⭐ Script de configuración inicial
├── run_server.sh               # ⭐ Ejecutar servidor
├── run_client.sh               # ⭐ Ejecutar cliente
├── GameServer/
│   ├── CoreGenericServer/      # Base del servidor (protocolo genérico)
│   ├── DonkeyKong/
│   │   ├── Game/               # Lógica del juego
│   │   ├── Server/             # Servidor específico DK
│   │   └── Client/             # Ejecutable del cliente
│   └── bin/                    # Clases compiladas
└── InterfaceCDevelpment/
    ├── Makefile                # Configuración de compilación
    ├── src/                    # Código fuente C
    │   ├── main.c
    │   ├── mapa.c
    │   └── Socket_client.c
    ├── include/                # Headers
    └── obj/                    # Objetos compilados
```

## 🛠️ Comandos Útiles

### Recompilar todo desde cero
```bash
bash setup.sh
```

### Solo recompilar el cliente
```bash
cd InterfaceCDevelpment
bash ../run_client.sh
```

### Solo recompilar el servidor
```bash
cd GameServer
javac -d bin CoreGenericServer/*.java DonkeyKong/Game/*.java DonkeyKong/Server/*.java
bash ../run_server.sh
```

### Limpiar archivos compilados
```bash
cd InterfaceCDevelpment
make clean

cd ../GameServer
rm -rf bin
```

## 🔌 Protocolo de Comunicación

El cliente y servidor se comunican mediante mensajes de texto:

| Tipo         | Formato                     | Ejemplo                     |
|--------------|-----------------------------|-----------------------------|
| **Conexión** | `CONNECT\|PLAYER\|<nombre>` | `CONNECT\|PLAYER\|JugadorC` |
| **Posición** | `POS\|<id>\|<x>\|<y>`       | `POS\|1\|100\|200`          |
| **Acción**   | `ACTION\|<id>\|<acción>`    | `ACTION\|1\|JUMP`           |
| **Salto**    |            -                | `ACTION\|1\|JUMP`           |
| **Liana**    |            -                | `ACTION\|1\|GRAB_LIANA`     |
| **Agua**     |            -                | `ACTION\|1\|WATER_RESPAWN`  |

## 🎮 Controles del Juego

| Tecla       | Acción                  |
|-------------|-------------------------|
| **← →**     | Mover izquierda/derecha |
| **↑ ↓**     | Subir/bajar en liana    |
| **ESPACIO** | Saltar                  |
| **Z**       | Agarrar/soltar liana    |
| **ESC**     | Salir del juego         |

## Notas para Colaboradores, gracias:

- El `setup.sh` debe ejecutarse **antes** de cualquier cambio en el código
- Las DLLs de Raylib se copian automáticamente en la compilación
- El cliente corre en modo **offline** si no hay servidor disponible
- Los logs de debug aparecen en la consola durante ejecución

## Troubleshooting importante; Errores comunes.

### Error: `gcc: command not found`
→ Ejecuta `bash setup.sh` o agrega `C:\msys64\mingw64\bin` al PATH

### Error: `javac: command not found`
→ Instala Java JDK y agrega al PATH

### Error: `libraylib.dll not found`
→ Ejecuta `bash setup.sh` nuevamente para copiar DLLs

### Cliente no se conecta al servidor
→ Asegúrate de ejecutar `bash run_server.sh` en otra terminal primero
