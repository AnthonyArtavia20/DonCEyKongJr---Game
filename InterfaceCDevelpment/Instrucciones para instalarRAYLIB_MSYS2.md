Instalar Raylib en Windows usando MSYS2 (MinGW64)

1) Descargar e instalar MSYS2
- Ve a https://www.msys2.org/ y descarga el instalador (64-bit).
- Instálalo (por ejemplo en `C:\msys64`).

2) Abrir la shell correcta
- Abre "MSYS2 MinGW 64-bit" desde el menú Inicio (muy importante usar la shell MinGW64).

3) Actualizar el sistema
```bash
pacman -Syu
# Si el proceso pide cerrar la terminal, cierra y vuelve a abrir "MSYS2 MinGW 64-bit"
pacman -Su
```

4) Instalar toolchain y Raylib
```bash
# instala gcc, make, raylib y pkg-config
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-raylib mingw-w64-x86_64-pkg-config
# o instalar paquetes mínimos
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-raylib mingw-w64-x86_64-pkg-config
```

5) Compilar tu proyecto
- Desde la misma shell `MSYS2 MinGW 64-bit` navega al directorio del proyecto:
```bash
cd /c/Users/VICTUS/Desktop/Proyecto/DonCEyKongJr---Game/Game
make
```
- El `Makefile` del repositorio fue parcheado para usar `pkg-config` automáticamente cuando esté disponible. Si `pkg-config` no está presente o no encuentra `raylib`, el `Makefile` usará la ruta de respaldo `C:/raylib/w64devkit`.

6) Problemas comunes
- "gcc: command not found": estás en la shell equivocada; abre "MSYS2 MinGW 64-bit".
- Errores de linking: asegúrate de instalar la versión `mingw-w64-x86_64-raylib`.
- Si usas otra instalación de raylib (por ejemplo raylib-w64devkit), ajusta `RAYLIB_PATH` en `Makefile`.

7) Recomendación
- Siempre compila desde la shell `MSYS2 MinGW 64-bit` para que `gcc`, `make` y `pkg-config` apunten a los binarios `mingw64`.
