# SoundFirst PRO - Manual de Usuario
*La guía intuitiva para dominar tu hardware.*

## ¿Qué es esto?
SoundFirst PRO es un **driver personalizado** que toma el control de tu teclado Komplete Kontrol (Serie A o Serie M) y lo convierte en una herramienta de producción profunda para Reaper.

A diferencia de los mapeos MIDI estándar, este driver "habla" directamente con Reaper. Sabe qué pista está seleccionada, qué plugin está abierto, y cambia instantáneamente el comportamiento de cada perilla y botón para adaptarse a tu tarea actual.

---

## El Concepto: 4 Modos
Piensa en tu teclado como un camaleón con 4 "personalidades" o Modos distintos. Cambias entre ellos dependiendo de lo que quieras hacer.

1.  **Modo Mixer (Por Defecto):** Para balancear niveles, panorama y transporte básico.
2.  **Modo Complemento (Plugin Mode):** Para ajustar plugins, sintetizadores y efectos.
3.  **Modo Audio:** Para editar formas de onda, cortar y fundir clips de audio.
4.  **Modo MIDI:** Para componer y editar notas en el editor MIDI.

Explorémoslos uno por uno.

---

## 1. Modo Mixer (Botón TRACK)
Esta es la base. Pulsa el botón **TRACK** para entrar en este modo.
Úsalo cuando estés balanceando tu mezcla o grabando.

### Las Perillas (Knobs)
*   **Toca una Perilla:** El sistema **accede al SOLO** de esa pista instantáneamente (si "Auto-Solo" está activado). Perfecto para chequear un instrumento específico en la mezcla sin hacer click.
*   **Gira Perillas 1-8:** Controla el **Volumen** del banco de 8 pistas seleccionado.
*   **Mantén SHIFT + Gira:** Controla el **Panorama** (Izquierda/Derecha) de las pistas.

### El Encoder (Joystick 4D)
*   **Girar:** Mueve el cursor de reproducción.
*   **Arriba/Abajo:** Selecciona la Pista Siguiente/Anterior. *Nota: Al moverte, las perillas se actualizan automáticamente para controlar el nuevo banco de pistas.*
*   **Izquierda/Derecha:** Salta al Marcador Anterior/Siguiente.
*   **Click:** Inserta un nuevo Marcador en la posición actual.

### Botones Vitales
*   **PLAY / REC / STOP:** Funcionan exactamente como esperas.
*   **LOOP:** Activa/desactiva el modo bucle (loop).
*   **METRO:** Enciende/apaga el metrónomo.
*   **TEMPO:** Golpéalo rítmicamente para marcar el BPM del proyecto (Tap Tempo).
*   **UNDO / REDO (Shift):** Corrige errores instantáneamente.
*   **MUTE / SOLO:** Silencia o pone en Solo la pista *actualmente seleccionada*.
*   **QUANTIZE:** Función Especial -> Pone en Solo la Pista de Referencia (Pista 1) para comparaciones A/B rápidas.

---

## 2. Modo Complemento (Botón PLUGIN)
Aquí ocurre la magia. Pulsa el botón **PLUGIN** para entrar.
Úsalo cuando quieras ajustar un EQ, Compresor o Sintetizador.

**¿Qué es el Modo Complemento?**
Este modo te permite controlar con tus perillas cualquier plugin VST/AU/CLAP que haya sido mapeado. Los mapeos pueden ser creados por el desarrollador (incluidos), por la comunidad para expandir la librería, o por ti mismo para tus herramientas favoritas.

> **Importante:** Este modo se enfoca en la *pista actualmente seleccionada*.

### Las Perillas (Knobs)
*   **Girar:** Controla un parámetro específico del plugin abierto (ej: Cutoff, Threshold, Gain).
*   **Tocar:** Si el plugin lo soporta (como Pro-Q3), tocar la perilla puede aislar (hacer solo) esa banda específica.

### El Encoder (Joystick 4D)
*   **Izquierda/Derecha:** Navega entre los diferentes plugins de la cadena (ej: moverte del EQ al Compresor).
*   **Click:** Abre o Cierra la ventana flotante del plugin actual.

### Navegación de Páginas (Botones PRESET)
Un plugin puede tener más de 8 parámetros.
*   **PRESET UP:** Ir a la Siguiente Página de 8 parámetros.
*   **PRESET DOWN:** Ir a la Página Anterior.

*Tip: Usa nuestra herramienta **SoundFirst PRO Mapper** para personalizar exactamente qué parámetro va a qué perilla.*
👉 **[Abrir la Guía de Mapeo Fácil (MAPPING_GUIDE_ES.md)](MAPPING_GUIDE_ES.md)**

---

## 3. Modo Audio (Browser -> Audio)
Pulsa **BROWSER** y selecciona "AUDIO" (o alterna) para entrar.
Úsalo cuando edites clips de audio en la línea de tiempo (vista Arrange).

### Las Perillas (Knobs)
En Modo Audio, las perillas se convierten en herramientas de edición de precisión para el *item de audio seleccionado*:
1.  **Perilla 1:** Mover Cursor (Precisión fina).
2.  **Perilla 2:** Nudge Item (Mover el clip ligeramente).
3.  **Perilla 3:** Recortar Inicio (Trim Start).
4.  **Perilla 4:** Recortar Final (Trim End).
5.  **Perilla 5:** Volumen / Ganancia del Item.
6.  **Perilla 6:** Longitud de Fade In.
7.  **Perilla 7:** Longitud de Fade Out.
8.  **Perilla 8:** Zoom Horizontal.

### Botones de Edición
*   **LOOP:** Corta (Divide) el item en la posición del cursor.
*   **QUANTIZE:** -
*   **Shift + QUANTIZE:** Invierte el item de audio (Reverse).

---

## 4. Modo MIDI (Browser -> MIDI)
Pulsa **BROWSER** y selecciona "MIDI" para entrar.
Úsalo cuando el Editor MIDI (Piano Roll) esté abierto.

### Las Perillas (Knobs)
1.  **Perilla 1:** Navegar entre notas de un acorde.
2.  **Perilla 2:** Pitch (Mover nota Arriba/Abajo).
3.  **Perilla 3:** Mover Nota (Izquierda/Derecha en el tiempo).
4.  **Perilla 4:** Longitud de Nota (Acortar/Alargar).
5.  **Perilla 5:** Transponer (Semitonos). *Shift: Octavas.*
6.  **Perilla 6:** Velocidad (Intensidad).
7.  **Perilla 7:** Tamaño de Rejilla (Grid).
8.  **Perilla 8:** Zoom.

---

## Herramientas Globales
Estas funcionan en todos los modos.

*   **SCALE:** Guarda el Proyecto (Ctrl+S).
*   **ARP:** Feedback Hablado -> Lee el Nivel de Pico (Peak) de la pista actual.
*   **IDEAS:** Feedback Hablado -> Lee la Reducción de Ganancia (GR) en compresores mapeados.

### Obtener Soporte
¿Necesitas ayuda? Verifica que tienes instaladas las extensiones SWS y OSARA requeridas.
Revisa la [Página de Releases](https://github.com/joseyxioly/SoundFirst-PRO/releases) para actualizaciones.

👉 **[Contáctame en Telegram](https://t.me/JoseyXioly)**
👉 **[Apoyar el Desarrollo / Comprame un Café](https://buymeacoffee.com/soundfirstpro)**

*Poder para los Creadores.*
