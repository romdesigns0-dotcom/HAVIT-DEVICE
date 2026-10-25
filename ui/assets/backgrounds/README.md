# Fondos de Figma

Archivo: `3bmX64I7Rt85DV3boqOlsc`, página `1X1`.

| Fuente original | Nodo | Conversión LVGL | Dimensiones |
| --- | --- | --- | --- |
| default_2.svg | 464:1361 | RGB565 | 360×360 |
| default_3.svg | 476:1876 | RGB565 | 360×360 |
| default_4.svg | 476:1869 | ARGB8888 | 360×360 |

Los SVG están descargados del contexto de diseño de Figma sin modificar.
`tools/generate_background_assets.py` produce los píxeles nativos, que ESP-IDF
enlaza mediante `EMBED_FILES`. No se necesitan URLs de Figma en ejecución.

Default 4 conserva el SVG original sin cambios. A petición del usuario, la
conversión para ejecución retira el velo blanco uniforme de alfa 51/255 que
lavaba el contraste: el centro pasa a transparente, manteniendo aro, borde
suavizado y máscara exterior negra. Se muestra en la capa superior fija de LVGL y no recibe
eventos táctiles. Default 2 contiene un aro más fino que el fondo de los ojos;
no deben compartir el mismo mapa de píxeles.
