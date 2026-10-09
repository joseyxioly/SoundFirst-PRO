# SoundFirst PRO - Controlador
![SoundFirst PRO Logo](logo.png)

**La Evolución "Sound-First" (El Sonido Primero).**

### Nacido de la Necesidad
Este proyecto nació de una necesidad real. Su creador, un productor invidente sin los recursos económicos para acceder a costoso hardware de accesibilidad, decidió no rendirse ante las barreras. En su lugar, creó su propio flujo de trabajo para nivelar el campo de juego.

Hoy, comparte esta herramienta libremente con el mundo, con la esperanza de que empodere a otros que enfrentan desafíos similares, o simplemente ayude a cualquier productor que busque una conexión más profunda y sin distracciones con su música.

> *"Cuando eliminas la distracción visual, lo único que queda es la verdad del audio."*

---

### Compatibilidad de Hardware Actual
*   **Native Instruments Komplete Kontrol Serie A**
*   **Native Instruments Komplete Kontrol Serie M**

*Con la ayuda de la comunidad, estamos construyendo un framework para agregar fácilmente soporte para otros controladores en el futuro.*

---

### Instrument Mode: Instrumentos NKS

Como muchos lo han pedido, **¡SoundFirst PRO ahora es compatible con instrumentos NKS!**

Toca tus instrumentos con los controles NKS nativos de Komplete Kontrol.  
Después vuelve a tu mezcla o edita el MIDI que acabas de grabar.

Instrument se suma como el quinto modo, junto a Mixer, Plugin, Audio y MIDI.

1. Selecciona y arma la pista que contiene Komplete Kontrol.
2. Pulsa **Browser**, gira el encoder para elegir **Instrument** y haz click en el encoder.
3. Pulsa **Plugin** y después **Browser** para controlar Komplete Kontrol.
4. Toca, explora presets o cambia de instrumento con los controles NKS nativos.
5. Pulsa **Browser dos veces** para salir de Instrument y volver a SoundFirst.
6. Desarma esa pista. Selecciona y arma la siguiente, y repite.

Con varias instancias de Komplete Kontrol, **la pista armada toma el control** en este flujo.  
Mantén esa misma pista seleccionada y solo esa pista armada.

Instrument deja que Native Instruments controle el instrumento.  
SoundFirst pausa sus controles personalizados hasta que salgas del modo.

---

### Instalación de la Nueva Versión

Cierra REAPER, quita `reaper_kompletereaper.dll` si está instalado y copia `reaper_soundfirst_pro.dll` a `UserPlugins`; después abre REAPER de nuevo.

---

### Por Qué Tu Estudio Necesita Esto

*   **Feel Analógico Real (10-Bits):** Olvídate de los "saltos" del MIDI estándar (0-127). Hemos desbloqueado la resolución nativa de los encoders (0-1023) para que sientas cada matiz de tus filtros y compresión.
*   **Tecnología Auto-Solo:** El sistema "sabe" lo que estás tocando. Posa tu dedo sobre una perilla de EQ y aislaremos esa frecuencia al instante. Levanta el dedo y vuelve a la mezcla. Sin clicks, sin menús.
*   **Velocidad HID Nativa:** Comunicación directa con el núcleo de Reaper a través de C++. Cero latencia. Confiabilidad total.
*   **Banking Inteligente:** El controlador rastrea tu selección en Reaper y asigna los faders automáticamente. Nunca más te perderás en bancos de 8 pistas.
*   **Mapeo Fácil:** Mapea cualquier plugin en segundos usando nuestro intuitivo **SoundFirst PRO Mapper**. Sin necesidad de programar.

---

### Apoya El Proyecto
Este software es de código abierto y libre. No pido nada a cambio, solo que lo pruebes.

Sin embargo, si aprecias este trabajo y mejora tu flujo de trabajo, por favor considera comprarme un café. Tu apoyo me ayuda a continuar desarrollando esta herramienta para todos.

👉 **[Contáctame en Telegram](https://t.me/JoseyXioly)**
👉 **[Apoyar el Desarrollo / Comprame un Café](https://buymeacoffee.com/soundfirstpro)**

---

### Documentación Completa
Para una referencia completa de cada botón, modo y función oculta:

👉 **[LEER EL MANUAL DE USUARIO (MANUAL_ES.md)](MANUAL_ES.md)**
👉 **[GUÍA DE MAPEO FÁCIL (MAPPING_GUIDE_ES.md)](MAPPING_GUIDE_ES.md)**

---

*(© 2026 José Pérez. Licencia GNU GPL v3 - Power to the creators.)*
