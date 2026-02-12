# SoundFirst PRO - Flujo de Trabajo VST Unificado 🎛️🧠

## La Filosofía: "Mezcla con Memoria Muscular"
SoundFirst PRO no es solo un driver; es un **flujo de trabajo**.
Creemos que para mezclar profesionalmente, no deberías tener que mirar la pantalla para encontrar la perilla de "Threshold" cada vez que abres un compresor nuevo.

**La Solución:** Un Flujo de Trabajo Unificado.
*   **La Perilla 1 (K1)** es *siempre* el **Driver** (Threshold, Input Gain, o Boost principal).
*   **La Perilla 2 (K2)** es *siempre* el **Nivel** (Makeup, Output, o Volumen).
*   **Shift + Perilla** accede a parámetros secundarios relacionados.
*   **Tocar Perilla 🖐️** a menudo pone en Solo el parámetro o banda (ej. en Pro-Q3).

Esto significa que puedes cerrar los ojos, cargar *cualquier* compresor soportado, y tu mano irá automáticamente a K1 para comprimir y K2 para ajustar ganancia. Tus manos aprenden la mezcla, no tus ojos.

> **Nota:** Este es un punto de partida profesional. Puedes personalizar CUALQUIER mapeo usando nuestras herramientas incluidas (carpeta `tools/`) para adaptarlo a tus necesidades. Pero te animamos a probar este enfoque unificado; lo cambia todo.

---

## Mapeos Incluidos (Nativos)

### 1️⃣ Channel Strips (Consolas)
*Emulaciones de consolas clásicas. El diseño está estandarizado entre marcas.*

**Lindell 50 / SSL EV2 / SSL 9000 J**
*   **Página 1 (EQ):**
    *   **K1-K4:** Ganancia Low, Low-Mid, High-Mid, High.
    *   **Shift + K1-K4:** Frecuencias de las bandas anteriores.
    *   **K5-K6:** Q (Ancho de banda).
    *   **K7-K8:** Tipos (Bell/Shelf) o Filtros.
*   **Página 2 (Dinámica):**
    *   **K1:** Threshold (El Driver).
    *   **K2:** Output / Makeup (El Nivel).
    *   **K3-K5:** Attack, Release, Ratio.
    *   **K6-K7:** Controles de Gate.

### 2️⃣ Compresores
*La regla "K1 = Threshold, K2 = Makeup" aplica siempre.*

**CLA-76 (Estilo 1176)**
*   **K1:** Input (Driver). *(Shift: Auto-Makeup)*
*   **K2:** Output (Nivel). *(Shift: Trim)*
*   **K3-K4:** Attack, Release.
*   **K5:** Ratio. *(Shift: Revisión/Color)*
*   **K6:** Mix (Paralelo).
*   **K7:** Ruido Analógico.

**CLA-2A (Estilo LA-2A)**
*   **K1:** Peak Reduction (Driver).
*   **K2:** Gain (Nivel).
*   **K3:** Hi-Freq (Sidechain).
*   **K4:** Modo Compress/Limit.

**SSL Bus Comp / API-2500 / Bettermaker / L3 Limiter**
*   **K1:** Threshold.
*   **K2:** Makeup / Ceiling.
*   **K3-K5:** Attack, Release, Ratio.
*   **K7-K8:** Funciones especiales (Thrust, Knee, THD, Profile).

**RVox (Waves)**
*   **K1:** Compresión.
*   **K2:** Ganancia.
*   **K3:** Gate.

### 3️⃣ Ecualizadores
*Desde EQs quirúrgicos hasta unidades pasivas vintage.*

**FabFilter Pro-Q 3**
*   **Página 1:** "Uso de Bandas" (Resumen).
*   **Página 2:** Bandas 1-4.
*   **Página 3:** Bandas 5-8.
    *   **Par de Perillas (ej. K1+K2):** Ganancia + Frecuencia.
    *   **Shift:** Q + Tipo.
    *   **Tocar:** Solo de Banda (Escuchar solo esa frecuencia).

**API-560 (EQ Gráfico)**
*   **K1-K8:** Controlan 8 bandas fijas por página.
    *   **Shift:** Accede a frecuencias intermedias o Ganancia de Salida.

**PuigTec EQP1A (Pasivo)**
*   **K1:** Low Boost.
*   **K2:** Low Atten.
*   **K3:** Low Freq.
*   **K5:** Hi Boost.
*   **K6:** Hi Freq.

**bx_digital V3 (EQ de Mastering M/S)**
*   **Página 1:** Canal Mid (Mono).
*   **Página 2:** Canal Side (Estéreo).
*   **K6 (Pág 1):** Mono-Maker.
*   **K6 (Pág 2):** Stereo Width (Ancho estéreo).

### 4️⃣ Creativos y FX
**H-Delay**
*   **K1:** Tiempo de Delay. *(Shift: Sync/BPM)*
*   **K2:** Feedback.
*   **K3-K4:** Filtros HP/LP.
*   **K5:** Mix.
*   **K7:** Ping-Pong On/Off.

**H-Reverb**
*   **K1:** Tiempo (Decay).
*   **K2:** Tamaño (Size).
*   **K3:** Pre-Delay.
*   **K5:** Dry/Wet.

**Black Box HG-2**
*   **K1:** Saturación (Pentode).
*   **K2:** Triodo.
*   **K5:** Input.
*   **K6:** Output.

---

## Personalizando tu Flujo
Esta lista es solo el comienzo.
Usando el **SoundFirst PRO Mapper** incluido en la carpeta `tools/`, puedes:
1.  Mapear *cualquier* otro plugin que tengas.
2.  Sobrescribir estos valores por defecto si prefieres otro diseño.
3.  Añadir "Acciones Inteligentes" (como `@REPORT_GR`) a tus propios mapeos.

**Únete a la Comunidad:** Estamos construyendo un estándar. ¡Comparte tus mapas `.ini` con otros usuarios para expandir esta librería!

👉 **[Contáctame en Telegram](https://t.me/JoseyXioly)**
👉 **[Apoyar el Desarrollo / Comprame un Café](https://buymeacoffee.com/soundfirstpro)**
