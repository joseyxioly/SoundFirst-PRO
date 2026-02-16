# SoundFirst PRO - Guía de Mapeo Fácil
*Mapea cualquier plugin en segundos, sin necesidad de programar.*

---

## El Concepto
SoundFirst PRO convierte tu Komplete Kontrol en un **controlador universal para cualquier plugin** VST/AU/CLAP.  
Usamos archivos `.ini` simples que indican al driver: *“La Perilla 1 controla Cutoff”*.  

La herramienta **SoundFirst PRO Mapper** hace que crear estos mapas sea instantáneo y visual, sin tocar código.

---

## Dónde Encontrar la Herramienta
* **Ejecutable (.exe):** Descarga el mapeador desde la sección de **[Releases](https://github.com/joseyxioly/SoundFirst-PRO/releases)**.  
* **Código fuente:** El código en Python está en la carpeta `tools/` del proyecto.

---

## Comunidad y Soporte
¿Tienes dudas o quieres compartir tus mapas?  
📢 [Únete al grupo de discusión en Telegram](https://t.me/SoundFirstPRO)  
☕ [Apoya el desarrollo / Cómprame un Café](https://buymeacoffee.com/soundfirstpro)

---

## Paso 1: Obtener los Parámetros del Plugin
1. Abre Reaper y agrega tu plugin a una pista (ej: *Serum* o *Pro‑Q3*).  
2. Haz click en la ventana del plugin para enfocarla.  
3. Abre la **Lista de Acciones** (`?`) y ejecuta **“Script: dump_fx_params.lua”**.  
   *Tip: asigna un atajo, por ejemplo `Ctrl+Shift+D`.*  
4. Copia el resultado al portapapeles (`Ctrl+C`).

---

## Paso 2: Abrir SoundFirst PRO Mapper
1. Ejecuta el archivo `SoundFirst_PRO_Mapper.exe`.  
2. Haz click en **“Create New Mapping”**.  
3. Pega el dump copiado del Paso 1 en la ventana que aparece y pulsa **“Next”**.  
4. Ya estás en el editor gráfico.

### Cómo Mapear
El editor tiene 3 columnas:

1.  **Columna Izquierda (Controles):** Selecciona la perilla o botón de tu hardware.
    *   *Opcional:* Marca **Shift** para la capa secundaria o **Touch** para acción al tocar (ej. Auto-Solo).
2.  **Columna Central (Parámetros):** Busca el parámetro que quieres controlar.
    *   *Ejemplo: escribe “Cutoff” en la caja de búsqueda.*
3.  **Asignar:** Presiona **“Assign (Enter)”** o simplemente la tecla **ENTER**.
    *   *La selección salta automáticamente al siguiente control para mapear rápido.*

### Funciones Avanzadas
- **Páginas:** Usa la columna derecha para **Add Page** (Añadir Página) si necesitas más de 8 perillas.
- **Click Derecho** en una página para renombrarla o reordenarla.

---

## Paso 3: Guardar y Usar
1. Haz click en **“Save Mapping”**.  
2. La herramienta sugerirá un nombre de archivo (ej: `Serum.ini`). **No lo cambies** salvo que sea necesario.  
3. Guarda el archivo en la carpeta que se abre por defecto (`.../UserPlugins/SoundFirst_Maps`).

> La próxima vez que abras ese plugin, SoundFirst PRO cambiará automáticamente a **Modo Complemento** y tus perillas funcionarán al instante.  
> ⚠️ **Nota:** No necesitas reiniciar Reaper; el driver lee los mapas dinámicamente.

---

*¡Feliz mapeo y a producir como un profesional!*
