# SoundFirst PRO V1.0 - Manual de Usuario 🎹🚀

Este manual explica cómo usar el controlador SoundFirst PRO de manera eficiente en **Reaper 7.x**, enfocándose en la lógica de los diferentes modos y el flujo de trabajo.

---

## 📑 Tabla de Contenidos
- [Navegación Global](#navegación-global-joystick-y-browser)
- [Modo Mixer (Botón TRACK)](#modo-mixer-botón-track)
- [Modo FX (Botón PLUGIN)](#modo-fx-botón-plugin)
- [Modo MIDI (Browser -> MIDI)](#modo-midi-browser---midi)
- [Modo Audio (Browser -> AUDIO)](#modo-audio-browser---audio)
- [Otros Botones Globales](#otros-botones-globales)
- [Descarga y Soporte](#descarga-y-soporte)

---

## 🕹️ Navegación Global (Joystick y Browser)

### Botón Browser
- Pulsa **BROWSER** para cambiar entre los modos **MIDI** y **AUDIO**.
- Confirma la selección haciendo click en el **Encoder**.

### Joystick (Encoder 4D)
- Las funciones cambian ligeramente dependiendo del modo actual pero siguen una lógica consistente:

| Acción | Modo Mixer | Modo FX | Modo MIDI / Audio |
| :--- | :--- | :--- | :--- |
| Giro | Mover cursor (1 beat) | Mover cursor | Mover cursor |
| Giro + SHIFT | Mover loop | Navegar transientes | Navegar transientes |
| Arriba | Pista anterior | Pista anterior | Pista anterior |
| Abajo | Pista siguiente | Pista siguiente | Pista siguiente |
| Izquierda | Marcador anterior | Plugin anterior | Item anterior |
| Derecha | Marcador siguiente | Plugin siguiente | Item siguiente |
| Click | Insertar marcador | Abrir/cerrar ventana FX | Abrir editor MIDI |
| Click + SHIFT | - | Cerrar ventanas flotantes | Cerrar editor/ventanas |

---

## 🎛️ Modo Mixer (Botón TRACK)
- Modo por defecto para mezcla y control de transporte.  
- Se activa pulsando el botón **TRACK**.

### Perillas (Knobs)
| Knob | Normal | SHIFT | Touch (Tocar) |
| :--- | :--- | :--- | :--- |
| 1-8 | Volumen de pista | Panorama (Pan) | Auto-Solo (si está activado vía botón IDEAS) |

### Transport & Editing Buttons
| Botón | Normal | SHIFT |
| :--- | :--- | :--- |
| PLAY | Play / Pause | Global Start (Ir al inicio) |
| REC | Grabar (Record) | Cycle Automation (Touch/Latch/Read) |
| STOP | Stop | - |
| LOOP | Toggle loop | - |
| METRO | Toggle metrónomo | - |
| TEMPO | Tap tempo | - |
| UNDO | Deshacer (Undo) | Rehacer (Redo) |
| QUANTIZE | Solo Referencia (Pista 1) | - |
| MUTE | Mute pista seleccionada | - |
| SOLO | Solo pista seleccionada | - |

---

## 🎹 Modo FX (Botón PLUGIN)
- Se activa pulsando el botón **PLUGIN**.  
- Permite un control profundo de los efectos en la pista seleccionada.

### Perillas (Knobs)
- Controlan parámetros mapeados del plugin (nativo o Auto-Map).  
- **Touch**: Si el plugin lo soporta (ej. FabFilter, Waves), tocar una perilla activa el **Touch Solo** para ese parámetro.

### Navegación de Parámetros
| Botón | Función |
| :--- | :--- |
| PRESET UP | Página siguiente (8 parámetros por banco) |
| PRESET DOWN | Página anterior |
| MUTE | Bypass plugin actual |
| IDEAS | Mostrar reducción de ganancia (dB) |

---

## 🎼 Modo MIDI (Browser -> MIDI)
- Edita notas y eventos MIDI con el editor MIDI abierto.

### Perillas (Knobs)
| Knob | Normal | SHIFT |
| :--- | :--- | :--- |
| 1 | Navegar acorde | Mantener selección |
| 2 | Pitch (nota arriba/abajo) | - |
| 3 | Mover evento (pixel) | Mover evento (grid) |
| 4 | Longitud de nota (pixel) | Longitud de nota (grid) |
| 5 | Transponer (semitono) | Octava |
| 6 | Velocidad (±1) | Velocidad (±10) |
| 7 | Tamaño de Grid | - |
| 8 | Zoom horizontal | - |

### Botones de Edición
| Botón | Acción |
| :--- | :--- |
| QUANTIZE | Cuantizar eventos |
| LOOP | Cortar (Cut) |
| METRO | Copiar (Copy) |
| TEMPO | Pegar (Paste) |
| PRESET UP | Dividir (Split) |
| PRESET DOWN | Borrar (Delete) |

---

## ✂️ Modo Audio (Browser -> AUDIO)
- Edita items de audio en la vista Arrange.

### Perillas (Knobs)
| Knob | Función |
| :--- | :--- |
| 1 | Mover cursor (fino) |
| 2 | Mover item (nudge por grid) |
| 3 | Recortar inicio (Trim start) |
| 4 | Recortar final (Trim end) |
| 5 | Volumen de Item |
| 6 | Fade In |
| 7 | Fade Out |
| 8 | Zoom horizontal |

### Botones de Edición
| Botón | Normal | SHIFT |
| :--- | :--- | :--- |
| QUANTIZE | Cuantizar items | Invertir (Reverse) |
| LOOP | Cortar (Cut) | - |
| METRO | Copiar (Copy) | - |
| TEMPO | Pegar (Paste) | - |
| PRESET UP | Dividir item (Split) | - |
| PRESET DOWN | Borrar / Eliminar | - |

---

## ⚙️ Otros Botones Globales
- **SCALE:** Guardar proyecto (Save)  
- **IDEAS:**  
  - Shift + IDEAS: Alternar medidor entre **PEAK** y **Reducción de Ganancia (GR)**  
  - Normal: Alternar Auto-Solo Global

---

## 📥 Descarga y Soporte
- **Releases / Descargar v1.0:** [Click aquí](https://github.com/joseyxioly/SoundFirst-PRO/releases)  
- **Apoyar el proyecto:** [Invítame a un café](https://www.buymeacoffee.com/soundfirstpro)

---

**© 2026 SoundFirst PRO**
