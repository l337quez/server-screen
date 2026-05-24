# Server Monitor & IA Agent Screen
![ESP32](https://img.shields.io/badge/ESP32-Espressif-%23E7352B.svg?style=for-the-badge&logo=espressif&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-%23F68220.svg?style=for-the-badge&logo=platformio&logoColor=white)
![LVGL](https://img.shields.io/badge/LVGL-v8.3.11-%2300A5FF.svg?style=for-the-badge&logo=lvgl&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-%2300979D.svg?style=for-the-badge&logo=arduino&logoColor=white)

Aplicación y base shell para la pantalla **ESP32 CYD (Cheap Yellow Display 2.8" - ESP32-2432S028R)** diseñada para actuar como un monitor en miniatura enfocado en servidores, mostrando:
- 🖥️ **Hardware del servidor**: Estado de CPU, memoria, almacenamiento y temperaturas en tiempo real.
- 🤖 **Agentes de IA activos**: Visualización de qué agentes están trabajando en el servidor y qué tareas específicas están procesando.
- 📊 **Estado del sistema**: Registro rápido de eventos, actividades del sistema y notificaciones críticas.

El proyecto está diseñado de forma modular, ofreciendo una pantalla principal limpia que resalta el logo de Claude y un botón de acceso a configuraciones con control de modo oscuro integral.

> [!NOTE]
> Este proyecto utiliza **LVGL v8.3.11** para lograr gráficos fluidos con suavizado y renderizado de alto rendimiento en hardware embebido.

---

## 📑 Tabla de Contenidos

- [⚡ Requerimientos](#-requerimientos)
- [🏗️ Estructura del Proyecto](#️-estructura-del-proyecto)
- [📦 Instalación y Preparación](#-instalación-y-preparación)
- [🚀 Compilación y Subida](#-compilación-y-subida)
- [⚙️ Configuración del Hardware (CYD)](#️-configuración-del-hardware-cyd)
- [🛠️ Detalles Técnicos: Inyección C99 de Logo](#️-detalles-técnicos-inyección-c99-de-logo)
- [🔐 Configuración de Ajustes y Preferencias](#-configuración-de-ajustes-y-preferencias)

---

## ⚡ Requerimientos

### Hardware
- **ESP32 CYD (Cheap Yellow Display)** - Modelo ESP32-2432S028R con pantalla resistiva ILI9341 de 320x240 y sensor LDR integrado.
- Cable Micro-USB para programación y alimentación.

### Software & Herramientas
![PlatformIO](https://img.shields.io/badge/PlatformIO-6.0+-F68220?logo=platformio&logoColor=white)
![Node Version](https://img.shields.io/badge/Node-v20+-339933?logo=node.js&logoColor=white)

---

## 🏗️ Estructura del Proyecto

```
/server-screen/
├── include/
│   ├── cyd_compat.h       # Cabecera de compatibilidad para descriptores de imagen LVGL v9
│   └── lv_conf.h          # Configuración del motor gráfico LVGL v8
├── src/
│   ├── icons/
│   │   └── claude.c       # Logotipo de Claude (5.8MB, compilado de forma independiente en C)
│   ├── main.cpp           # Loop principal, inicialización de periféricos y pantalla principal
│   ├── settings.h         # Declaración de la pantalla de ajustes
│   └── settings.cpp       # Pantalla de ajustes con pestañas (System -> Dark Mode)
└── platformio.ini         # Configuración del entorno de compilación y dependencias
```

---

## 📦 Instalación y Preparación

1. Clona o abre esta carpeta en tu editor de código preferido (VS Code recomendado con la extensión de **PlatformIO IDE**).
2. Asegúrate de que el archivo del logotipo `claude.c` esté ubicado en la carpeta `src/icons/`.
3. PlatformIO leerá automáticamente el archivo `platformio.ini` al abrir el proyecto y descargará las dependencias necesarias.

---

## 🚀 Compilación y Subida

> [!IMPORTANT]
> Asegúrate de tener conectado tu módulo ESP32 CYD por puerto USB antes de proceder con la subida.

### Utilizando PlatformIO en VS Code
- **Compilar**: Haz clic en el botón de **Build** (✓) en la barra inferior de VS Code.
- **Subir**: Haz clic en el botón de **Upload** (→) en la barra inferior.

### Utilizando la CLI de PlatformIO
```bash
# Compilar el proyecto
$ pio run

# Compilar y subir directamente al ESP32
$ pio run -t upload

# Abrir el monitor serial para depurar
$ pio device monitor
```

---

## ⚙️ Configuración del Hardware (CYD)

La pantalla Cheap Yellow Display (CYD) utiliza un mapeo de pines SPI específico. En este proyecto, todo se configura en tiempo de compilación a través de las `build_flags` en `platformio.ini` para evitar modificar archivos de librerías globales:

| Periférico | Pin ESP32 | Flag Configuración | Función |
|------------|-----------|--------------------|---------|
| **TFT CS** | `15` | `-D TFT_CS=15` | Chip Select de la Pantalla |
| **TFT DC** | `2` | `-D TFT_DC=2` | Data/Command de la Pantalla |
| **TFT RST** | `-1` | `-D TFT_RST=-1` | Reset de Pantalla (compartido) |
| **Touch CS** | `33` | `-D TOUCH_CS=33` | Chip Select del Panel Táctil |
| **Backlight** | `21` | `-D TFT_BL=21` | Control de Brillo (PWM en Canal 6) |
| **LDR Sensor** | `34` | *Definido en código* | Sensor de Luz para brillo automático |

---

## 🛠️ Detalles Técnicos: Inyección C99 de Logo

> [!TIP]
> **Compatibilidad LVGL v9 → v8 sin tocar archivos gigantes**:
> El archivo `claude.c` (5.8 MB) contiene el logo oficial con un formato de inicializador designado anidado exclusivo de LVGL v9 (ej. `.header.cf = ...`). C++ no soporta esta sintaxis, lo que causaría fallos de compilación si se incluyera directamente en archivos `.cpp`.

Para solucionarlo de forma limpia:
1. Compilamos `claude.c` de forma separada en `src/icons/claude.c` utilizando el compilador nativo de C (el cual soporta inicializadores C99 a la perfección).
2. Utilizamos la bandera `-include cyd_compat.h` en `platformio.ini` para inyectar la compatibilidad a nivel de preprocesador al inicio del archivo sin tocar el código de `claude.c`.
3. Mapeamos el campo `magic` de LVGL v9 directamente sobre el campo de bits de 3 bits `always_zero` en el encabezado de LVGL v8, previniendo advertencias de desbordamiento (`overflow`).
4. Enlazamos el símbolo de forma segura en C++ usando:
   ```cpp
   extern "C" const lv_img_dsc_t HF4bjm7aMAAeTwB;
   ```

---

## 🔐 Configuración de Ajustes y Preferencias

El proyecto cuenta con un sistema persistente en la memoria Flash no volátil (`NVS`) del ESP32 a través de la librería `Preferences`.

| Clave | Tipo | Valor por Defecto | Descripción |
|-------|------|-------------------|-------------|
| `"dark"` | `bool` | `true` | Alterna entre Modo Oscuro (Fondo Oscuro) y Modo Claro (Fondo Claro). |

> [!WARNING]
> Al cambiar el estado de **Dark Mode** en la pestaña **System**, la placa realizará un `ESP.restart()` para reconstruir el tema global de LVGL con los colores inversos corregidos de la pantalla.

---

## 🎨 Ajuste y Corrección del Color

> [!NOTE]
> Las pantallas SPI ILI9341 de la CYD requieren habilitar el intercambio de bytes (`LV_COLOR_16_SWAP = 1`) en `lv_conf.h` para que los canales de color Rojo y Azul se transmitan en el orden correcto a través del bus SPI. Esto evita que los colores tengan una capa azulada, mostrando el naranja oficial de Claude de forma vívida y natural.

---

## 📝 Licencia

Este proyecto es de código abierto. Desarrollado de forma personalizada e integrada para pantallas de monitorización de servidores embebidos.