# Fondos Default de HAVIT

Los fondos de Figma se organizan como capas con funciones distintas:

- **Default 1 — ojos:** visible durante la introducción animada.
- **Default 2 — aplicación:** se muestra al terminar la introducción y permanece
  como fondo de Login 1 y de las demás vistas actuales y futuras.
- **Default 3 — base desplazable:** rectángulo negro inferior en formularios que
  necesitan scroll vertical.
- **Default 4 — máscara desplazable:** aro degradado superior y exterior negro
  opaco. El SVG original tiene un velo blanco de alfa 51/255 en el centro; la
  conversión de ejecución lo retira a petición del usuario para recuperar el
  negro y el contraste, conservando el original sin modificar.

Al cerrar el parpadeo final, `eyes_intro.c` oculta la capa Default 1 y revela
Default 2 antes de mostrar Login 1. Default 1 conserva el recurso de los ojos;
Default 2 usa su SVG original, cuyo aro es más fino. Default 2/3/4 están
convertidos a píxeles de 360×360 y enlazados desde flash; no reservan otra copia
en PSRAM. Para Teams se oculta Default 2 y se coloca Default 4 en `layer_top`,
encima de todos los hijos de la pantalla, sin capturar toques. Al salir se oculta
Default 4 y se restaura Default 2. Mover una vista al frente durante la transición
ya no puede ponerla encima de la máscara.
La selección se realiza en el punto común de navegación antes de iniciar cada
transición, para que no dependa del tipo de animación.

Fuentes originales: `ui/assets/backgrounds/default_2.svg` (Figma `464:1361`),
`default_3.svg` (`476:1876`) y `default_4.svg` (`476:1869`). La conversión se
reproduce con `tools/generate_background_assets.py` (Pillow y resvg-py).

## Validación

- ESP-IDF 5.4.2 / LVGL 9.5: compilación correcta. Firmware de 3,026,848 bytes;
  76 % libre en la partición de aplicación de 12 MiB.
- Carga a COM5 correcta y hashes verificados. Se confirmó el arranque con
  ST77916, CST816S y 8 MiB de PSRAM. A los 31 segundos había 244,071 bytes
  internos libres y 6,812,592 bytes de PSRAM libres.
- Default 4 se lee desde flash y ya no reserva su copia anterior de 518,400
  bytes en PSRAM.
- El registro confirmó Default 2 en Login/Today y Default 3 + contenido +
  Default 4 al navegar al formulario Teams.
- LVGL validó el destino táctil del viewport y un recorrido vertical de 731 px.
  El gesto real llegó hasta `scroll end: y=731` y permitió accionar Generar.
- La escritura solo borró sectores del bootloader, aplicación y tabla de
  particiones. NVS, que empieza en `0x9000`, no se borró.
- El usuario confirmó mejora de contraste y desplazamiento tras retirar el
  velo central y aplicar la caché. Detalles en `docs/RENDERIZADO_SCROLL.md`.
