# Komplete Kontrol A61 - Reaper PRO V1.0 (Manual Oficial) 🎹🚀

Este documento es la **VERDAD ABSOLUTA (Ground Truth)** sobre el funcionamiento del controlador en la versión V1.0 (Public Release).

---

## ⚠️ Requisitos del Sistema
Para que este driver funcione correctamente (especialmente la navegación y accesibilidad), NECESITAS tener instalado:

1.  **Sistema Operativo:** Windows 10 o 11 (64-bits). *Aún no compatible con Mac/Linux.*
2.  **Reaper:** Versión 6.x o 7.x (Recomendado última versión).
3.  **SWS Extensions:** OBLIGATORIO. (Descargar en sws-extension.org).
4.  **OSARA:** OBLIGATORIO. El driver usa comandos de OSARA para la navegación de items y accesibilidad. Sin OSARA, botones como "Item Anterior/Siguiente" no funcionarán.
5.  **Visual C++ Redistributable:** (Normalmente ya lo tienes, pero si falla, instálalo).

---

## 🏗️ Filosofía de Modos
El sistema tiene 4 Modos Principales. Para cambiar entre ellos:
*   **BOTÓN BROWSER:** Alterna únicamente entre **MIDI** y **AUDIO**.
*   **BOTÓN TRACK:** Activa directamente el modo **MIXER**.
*   **BOTÓN PLUGIN:** Activa directamente el modo **FX**.

---

## 🕹️ El Joystick (Encoder 4D)

El comportamiento del Joystick cambia según el modo activo:

| Acción | MODO MIXER (Global) | MODO FX (Plugins) | MODO MIDI / AUDIO |
| :--- | :--- | :--- | :--- |
| **Giro** | Navegar Pistas (Select) | Navegar Pistas | Navegar Pistas |
| **Arriba** | Pista Anterior | Pista Anterior | Pista Anterior |
| **Abajo** | Pista Siguiente | Pista Siguiente | Pista Siguiente |
| **Izquierda** | **Marcador Anterior** | **Plugin Anterior** (Cadena) | **Item Anterior** (Silencioso)* |
| **Derecha** | **Marcador Siguiente** | **Plugin Siguiente** (Cadena) | **Item Siguiente** (Silencioso)* |
| **Click** | (Sin acción) | Abrir/Cerrar Ventana FX | Abrir Editor / Zoom Item |

*\*Silencioso = No habla "Item Anterior" para no pisar a OSARA.*

---

## 🎛️ Modos y Funciones

### 1. MODO MIXER (Botón Track)
Control total de la mezcla y transporte.

*   **Knobs 1-8:** Volumen del Track (Shift = Pan).
*   **Touch K1-8:** **Auto-Solo** (Si está activado en IDEAS).
*   **Ideas:** Toggle **Global Auto-Solo** (Activa/Desactiva que al tocar perillas se haga Solo).
*   **Scale:** **Guardar Proyecto**.
*   **Quantize:** Solo Pista 1 (Referencia). 2ª vez = Undo.
*   **Loop:** Toggle Loop (Repeat).
*   **Metro:** Toggle Metrónomo.
*   **Tempo:** Tap Tempo.
*   **Play/Stop/Rec:** Transporte estándar.
*   **Undo/Redo:** Undo (Shift = Redo).
*   **Mute/Solo:** Mute/Solo Pista Seleccionada.

### 2. MODO FX (Botón Plugin)
Control profundo de plugins con mapeos nativos.

*   **Knobs 1-8:** Controlan parámetros del plugin (ver lista abajo).
*   **Touch K1-8:** **Touch Solo** (Solo de banda EQ en FabFilter, Waves F6, etc).
*   **Preset UP:** **Página Siguiente** (Bancos de 8 parámetros).
*   **Preset DOWN:** **Página Anterior**.
*   **Ideas:** **Report Gain Reduction** (Si el plugin lo soporta).
*   **Mute:** **Bypass Plugin** (Activa/Desactiva el efecto).
*   **Joystick Izq/Der:** Navegar entre plugins de la cadena.
*   **Joystick Click:** Abrir/Cerrar ventana flotante del plugin.

### 3. MODO MIDI (Botón Browser -> MIDI)
Edición y navegación de items MIDI.

*   **Knob 7:** Grid (Invertido: Der = más fino).
*   **Knob 8:** Zoom Horizontal.
*   **Joystick Izq/Der:** Seleccionar Item Anterior/Siguiente.
*   **Preset UP:** Dividir Item (Split).
*   **Preset DOWN:** Borrar Eventos / Item.
*   **Loop:** Cortar (Cut).
*   **Metro:** Copiar (Copy).
*   **Tempo:** Pegar (Paste).
*   **Quantize:** Cuantizar Eventos.
*   **Shift + Joystick Click:** Cerrar ventana MIDI Editor.

### 4. MODO AUDIO (Botón Browser -> AUDIO)
Edición rápida de audio.

*   **Joystick Izq/Der:** Seleccionar Item Anterior/Siguiente.
*   **Preset UP:** Dividir Item (Split).
*   **Preset DOWN:** Borrar Item.
*   **Loop:** Cortar (Cut).
*   **Metro:** Copiar (Copy).
*   **Tempo:** Pegar (Paste).
*   **Quantize:** Reverse Item (Invertir audio).

---

## 🔌 Plugins Soportados (Mapeos Nativos)

### Consolas & Channel Strips
*   **Lindell 50 / 80 / 6X-500**
*   **SSL EV2 Channel (Waves)**
*   **bx_console SSL 9000 J**
*   **API 550A / 560 / 2500**

### Ecualizadores
*   **FabFilter Pro-Q 3/4:**
    *   **Página 1:** Activar/Desactivar Bandas.
    *   **Página 2:** Bandas 1-4 (Gain, Freq, Q - Touch Solo).
    *   **Página 3:** Bandas 5-8 (Gain, Freq, Q - Touch Solo).
*   **Waves F6:** Completo con Touch Solo.
*   **PuigTec EQP1A:** Mapeo clásico Pultec.
*   **bx_digital V3:** Mapeo M/S completo.

### Compresores
*   **CLA-76 (Bluey/Blacky)**
*   **CLA-2A**
*   **SSL Bus Compressor**
*   **Bettermaker Bus Compressor**
*   **RVox**

### Otros
*   **H-Delay / H-Reverb**
*   **Black Box HG-2**
*   **L3 UltraMaximizer**

---

## 🛠️ Instalación

### Opción A: Usuarios (Recomendado)
No necesitas compilar nada.
1.  Ve a la pestaña **"Releases"** (a la derecha en GitHub).
2.  Descarga el archivo `reaper_komplete_pro.dll`.
3.  Cópialo en tu carpeta de plugins de usuario de Reaper:
    *   `%APPDATA%\REAPER\UserPlugins\`
    *   (Normalmente es `C:\Users\TU_USUARIO\AppData\Roaming\REAPER\UserPlugins`)
4.  Reinicia Reaper. ¡Listo!

### Opción B: Desarrolladores (Compilar)
Si quieres modificar el código:
1.  **Requisitos:** CMake, Visual Studio Build Tools.
2.  **Carpeta:** `C:\Users\josep\.gemini\antigravity\scratch\KompleteReaper PRO V3`
3.  **Compilar:**
    ```powershell
    cmake --build . --config Release
    ```

---

**© 2026 Jose & Gemini Antigravity Team**
