# Flujo de Equipos

Referencia visual: Figma `1X1`, nodo `464:201` y frames siguientes del flujo.

## Recorrido

1. `Today` → botón `+` → `Agregar` → `Tarea en Equipo` → formulario de creación.
2. Formulario de 360×1091 px → desplazarse verticalmente → `Generar fechas de entrega`.
3. Pantalla de generación con indicador animado → avanza automáticamente a la línea de tiempo.
4. Línea de tiempo → tocar la flecha inferior → `Desglose`.
5. `Desglose` → tocar la marca de confirmación → `Enviar a Equipo`.
6. Tocar `Enviar a Equipo` → vuelve a `Today`.
7. La X del formulario regresa a `Agregar`.

Las pantallas cortas usan el fundido de 300 ms existente. El formulario permite
desplazamiento vertical nativo de LVGL; el controlador sondea el CST816S mientras
el dedo está apoyado para actualizar las coordenadas del arrastre. Al volver a
abrirlo, inicia arriba. En esta vista se ordenan las capas como Default 3 negro,
formulario y Default 4 (aro y exterior negro). Su centro se deja transparente
en ejecución a petición del usuario para conservar el contraste.
El viewport tiene `CLICKABLE`, `SCROLLABLE` y `PRESS_LOCK`: sin `CLICKABLE`, la
prueba de impacto de LVGL lo descarta y ningún arrastre puede empezar en él.
X y Generar se procesan mediante `LV_EVENT_CLICKED`, al soltar sin haber hecho
scroll, en lugar del toque inicial. Se registran inicio y fin de cada scroll.

## Alcance de esta etapa

Se integró la composición visual de Figma y la navegación de prototipo. Los campos,
miembros, fechas, desglose y envío son contenido de muestra: todavía no guardan
datos ni generan fechas mediante un servicio real. NVS no se borra ni modifica.

Se habilitó `CONFIG_LV_USE_ARC` para el indicador animado de la pantalla `Generando`.

## Prueba física

1. Desde `Today`, toca `+` y después `Tarea en Equipo`.
2. Desliza el dedo hacia arriba para ver el final y el botón inferior; hacia
   abajo para regresar al inicio. Toca `Generar fechas de entrega`.
3. Espera a que termine la animación; aparecerá el calendario de la línea de tiempo.
4. Toca la flecha inferior, confirma el desglose y toca `Enviar a Equipo`.
5. Verifica que vuelva a `Today`. Reabre el menú `Agregar` y confirma que la X del
   formulario regrese a esa pantalla.

El formulario se contrastó con el contexto actual de Figma `464:201`; se
actualizaron posiciones y tamaños y se sustituyeron sus dibujos aproximados
por recursos originales. Siete recursos se comparten con Today/Tarea Rápida
tras comprobar dimensiones y píxeles idénticos. Los objetos se conservan como
fuente de una caché RGB565 preparada al arrancar; no se reconstruyen al arrastrar.

## Última actualización

ESP-IDF 5.4.2 compiló correctamente. El firmware de 3,026,848 bytes se cargó a
COM5 con verificación de hash. El monitor serial confirmó arranque, 8 MiB de
PSRAM, ST77916 y CST816S. La selección de fondos se centralizó para que todas
las transiciones activen Default 2 en las pantallas normales y Default 3 +
contenido + Default 4 en el formulario desplazable. Se sustituyó el dibujo
compartido de Default 1 por los recursos originales 2/3/4 de Figma. Default 4
vive en la capa superior, de modo que las vistas no pueden cubrirlo al pasar
al frente. La NVS (`0x9000`) no se borró.

El arranque ejecutó la prueba de impacto de LVGL sobre el formulario real y
confirmó que captura el puntero y tiene 731 px de rango vertical. El registro de
una interacción física confirmó que llegó hasta y=731 y abrió Generar. También
se registró el resto del flujo hasta Today, sin reinicios. El usuario confirmó
que mejoraron tanto el contraste como el desplazamiento. Memoria y tiempos
medidos se documentan en `docs/RENDERIZADO_SCROLL.md`.

## Revisión de tipografía y geometría — 18/10/2026

Se consultaron nuevamente los cinco frames originales mediante Figma MCP,
incluidas sus capturas y posiciones. `font_at` enviaba los tamaños 10, 11,
13, 14 y 15 al caso predeterminado de 24 px: títulos y descripciones del
calendario/desglose se agrandaban y recortaban. Ahora cada tamaño tiene su
fuente Instrument Sans; también se incluyen 5 y 7 px para los detalles del
desglose tal como están en Figma. Son tamaños pequeños a revisar físicamente.

Las etiquetas conservan la altura natural del glifo y se centran dentro del
interlineado de Figma; las dos líneas de generación y título del calendario
son explícitas. Se ajustaron textos, coordenadas, colores y posición de días,
encabezados y fechas. El título «Crear Equipos» y los textos del formulario
respetan las coordenadas originales. «Entrega final» amplía su caja 2 px,
manteniendo el centro, porque el ancho rasterizado es 95 px frente a 94 en Figma.

Se descargaron los 16 SVG de las otras cuatro vistas. Sus iconos de carga,
calendario, desglose y envío sustituyen los dibujos aproximados. Los 29 SVG
del módulo tienen archivo local no vacío, dimensiones nativas y referencia
en el código. El formulario conserva su caché 360×1091 y rango de 731 px.

Compilación ESP-IDF completada sin errores: 3,195,024 bytes, 75% libre en la
partición de aplicación. Carga en COM5 con verificación de hash, sin borrar
NVS. El arranque confirma pantalla, táctil y caché; la comprobación final
de aspecto, centrado y legibilidad de las cinco vistas requiere prueba física.
