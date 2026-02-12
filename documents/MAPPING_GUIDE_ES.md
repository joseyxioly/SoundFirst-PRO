# SoundFirst PRO - Guía de Mapeo Fácil
*Mapea cualquier plugin en segundos, sin código.*

## El Concepto
SoundFirst PRO te permite controlar cualquier plugin VST/AU/CLAP con tu hardware Komplete Kontrol.
Para hacer esto, creamos archivos `.ini` simples que le dicen al driver: *"La Perilla 1 controla el Cutoff"*.

Hemos construido una herramienta especial llamada **SoundFirst PRO Mapper** para hacer esto instantáneo.

### Dónde Encontrar la Herramienta
*   **Ejecutable (.exe):** Descarga el último `SoundFirst_PRO_Mapper.exe` desde la sección de **[Releases](https://github.com/tu_usuario/tu_repo/releases)** de este repositorio.
*   **Código Fuente:** El código fuente completo en Python está disponible en la carpeta `tools/` de este proyecto.

### Comunidad y Soporte
¿Necesitas ayuda para mapear? ¿O quieres compartir tus mapas?
👉 **[Contáctame en Telegram](https://t.me/JoseyXioly)**
👉 **[Apoyar el Desarrollo / Comprame un Café](https://buymeacoffee.com/soundfirstpro)**

---

---

## Paso 1: Obtener los Parámetros
Primero, necesitamos saber qué parámetros tiene tu plugin.

1.  Abre Reaper y agrega el plugin que quieres mapear (ej: *Serum* o *Pro-Q3*) a una pista.
2.  Asegúrate de que la ventana del plugin esté enfocada (haz click en ella).
3.  Abre la Lista de Acciones (`?`) y ejecuta **"Script: dump_fx_params.lua"**.
    *   *Tip: ¡Asigna un atajo como `Ctrl+Shift+D` a este script!*
4.  Un mensaje confirmará: **"Parameters copied to clipboard!"**.

---

## Paso 2: La Herramienta Mapper
Ahora, abre la herramienta **SoundFirst PRO Mapper** incluida en la carpeta de descarga (`run_fx_mapper.cmd`).

1.  **Seleccionar Acción:** Elige **"Create New Mapping"**.
2.  **Pegar:** Haz click en "Next". La herramienta leerá automáticamente el portapapeles del Paso 1.
3.  **Wizard:** Ahora estás en el editor gráfico.

### Cómo Mapear
La interfaz está diseñada para la velocidad:

*   **Columna Izquierda (Controles):** Selecciona la Perilla o Botón en tu hardware.
    *   *Ejemplo: Selecciona "Knob 1".*
*   **Columna Central (Parámetros):** Busca el parámetro que quieres.
    *   *Ejemplo: Escribe "Cutoff".*
*   **Asignar:** Presiona **ENTER**.
    *   ¡Listo! La Perilla 1 ahora está vinculada al Cutoff.
    *   La selección salta automáticamente a "Knob 2" para que puedas seguir mapeando sin tocar el ratón.

### Funciones Avanzadas
*   **Páginas:** Puedes agregar múltiples páginas de controles (Página 1, Página 2...). Usa el botón "Add Page".
*   **Etiquetas Personalizadas:** Puedes renombrar parámetros para que aparezcan mejor en la pantalla (ej: "FiltFreq" en lugar de "Filter Frequency A").
*   **Específicos del Modo:**
    *   **Strip (Touch):** Si mapeas un parámetro a "Touch", tocar la perilla activará el Auto-Solo para esa banda.

---

## Paso 3: Guardar y Tocar
1.  Haz click en **"Save Mapping"**.
2.  La herramienta sugerirá un nombre de archivo (ej: `Serum.ini`). **No cambies este nombre** a menos que sea necesario, ya que el driver lo necesita para reconocer el plugin.
3.  Guárdalo en tu carpeta `UserPlugins\FX_Mappings`.
4.  **Reinicia Reaper** (o recarga el driver).

La próxima vez que abras ese plugin, SoundFirst PRO cambiará automáticamente a **Modo Complemento** y tus perillas funcionarán al instante.

---

## ¿Dónde pongo los archivos?
*   **Windows:** `%APPDATA%\REAPER\UserPlugins\FX_Mappings\`
*   **Mac:** `~/Library/Application Support/REAPER/UserPlugins/FX_Mappings/`

*¡Feliz Mapeo!*
