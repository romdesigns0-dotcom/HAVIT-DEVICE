# Today, Agregar y calendario

Referencia Figma: archivo `3bmX64I7Rt85DV3boqOlsc`, página `1X1`.
Frames `364:13` (Today), `364:46` (Agregar), `364:260` (mes),
`364:384` (detalle). Diseño de 360 × 360. Se conserva el fondo común.

## Corrección de geometría y tipografía

La primera versión ignoraba el tamaño de fuente solicitado y limitaba
la etiqueta de tareas a 25 px aunque la fuente de 22 px ocupa 29 px de alto.
También centraba las etiquetas de los botones en toda la pantalla,
ubicaba el calendario en el centro y construía iconos con trazos aproximados.
Los nombres de actividades estaban dentro de círculos que los recortaban.

Se sustituyeron esas aproximaciones por los SVG originales de Figma,
rasterizados fuera del ESP32 como imágenes LVGL ARGB8888. Se conservan
las fuentes SVG en `ui/assets/today/`. No se utiliza ThorVG en estas vistas.
El renderizador resvg conserva las máscaras y los degradados de los originales.
Los recursos iguales se comparten: 24 imágenes únicas de 32 recursos fuente.

Los cuatro botones orbitales de Today tienen 67 px. Centros:
(180.5, 64.5), (58.5, 179.5), (302.5, 179.5), (180.5, 294.5).
El círculo central mide 117 px y el borde 129 px. El aro mide 249 × 248 px.
Instrument Sans: Hoy 32 px, fecha 12 px, tareas 24 px. Las etiquetas usan
altura automática y alineación de la caja de línea de Figma; el fondo se
crea antes del texto. Los nombres del detalle son etiquetas independientes
de los círculos. Se corrigieron también los tamaños de cierre y más del detalle.

## Navegación de demostración

- Bienvenida espera 800 ms una vez completamente visible y abre Today
  automáticamente, con Smart Animate, Ease In Out y duración de 300 ms.
  Los toques en Bienvenida no adelantan el temporizador.
- Más en Today abre Agregar. X vuelve a la vista que abrió Agregar.
- Calendario a la derecha de Today abre el mes.
- Un día del mes abre su detalle. X vuelve al mes.
- X del mes vuelve a Today. Este control se añadió por petición del usuario;
  el frame mensual de Figma no lo incluye.
- Más del detalle abre Agregar y su X devuelve al detalle.

Los textos y días son objetos LVGL editables. Los puntos y cantidades usan
una misma tabla de datos de demostración basada en los días marcados de Figma.
No se han conectado fecha real, creación de tareas ni almacenamiento de actividades.
Los botones de tipos de tarea y las flechas de cambio de mes aún no ejecutan servicios.
Today ↔ Agregar usa una transición por componentes de 300 ms con Ease Out
(`cubic-bezier(0, 0, 0.58, 1)`, ruta nativa de LVGL 9.5). Los elementos
coincidentes conservan su posición: aro, círculo central, Hoy, contador y
dos círculos verdes. Los colores de los botones derecho e inferior se
interpolan por composición; los iconos, la fecha y las opciones de tarea
cambian por opacidad. No se crean ni destruyen objetos durante la transición.
Los toques se bloquean durante los 300 ms para evitar entradas duplicadas.
Se aplica el mismo efecto al cerrar Agregar con X hacia Today.
Para páginas con diseños distintos se utiliza Dissolve, Ease Out, 300 ms.
Se aplica a Today ↔ calendario mensual, calendario mensual ↔ detalle y
detalle ↔ Agregar. Se anima la opacidad de ambas vistas con un único reloj,
sin desplazar o escalar elementos. El destino se coloca delante del origen
para obtener el mismo resultado al avanzar y al regresar con X. El fondo
común permanece fijo y el tacto se desbloquea al terminar.
Bienvenida → Today tiene la excepción indicada por el prototipo: Smart
Animate con Ease In Out. Se conserva el fondo coincidente y se animan las
opacidades de los textos y componentes que no tienen correspondencia entre
los dos frames. No se agregan desplazamientos o escalas sin referencia.

## Reproducción

Los archivos C de imágenes y fuentes están incluidos en el proyecto y se
compilan sin herramientas de conversión adicionales.
Para regenerar imágenes: instalar `Pillow` y `resvg-py==0.5.0`, luego
ejecutar `python tools/generate_today_assets.py`.
Para regenerar las seis fuentes: `tools/generate_today_fonts.ps1`
(Node y `lv_font_conv@1.5.3`, alfabeto ASCII y Latin-1 con acentos).

## Validación de esta corrección

Se revisaron los recursos rasterizados en una previsualización local de
Today y Agregar contra los renders de Figma. La previsualización comprueba
geometría y recursos; no sustituye la revisión visual en la pantalla física.
La compilación ESP-IDF terminó sin errores: 2.134.928 bytes (`0x209390`),
83 % libre en la partición de aplicación de 12 MiB.
El aumento frente a la primera versión incluye los recursos originales y
los tamaños de fuente nuevos.

Prueba física: recorrer ojos → Login → Bienvenida → Today; comprobar
"Hoy", fecha y "0 tareas" completos, abrir Agregar y volver con X,
abrir el calendario con el botón derecho, seleccionar el día 11 y comprobar
las tres actividades y sus nombres; volver con las X.
La carga utiliza COM5 sin `erase-flash` y sin escribir NVS.
Esptool verificó los hashes de escritura. La captura serie posterior registró
LVGL activo a los 40 y 50 segundos con 289.247 bytes de heap interno libre,
sin reinicios ni errores de integridad durante esa captura. Se cerró COM5
al terminar. Esta observación no valida todavía las transiciones nuevas bajo uso.
La confirmación visual y del recorrido por el usuario queda pendiente.

## Transición de Agregar: 300 ms, Ease Out

Se compiló y cargó la transición por componentes en COM5 con hashes
verificados. Binario: 1.766.400 bytes (`0x1af400`), 86 % libre en la
partición. La separación de los círculos de Today permite compartir sus
recursos con Agregar y elimina del binario la imagen compuesta anterior.
La captura registró un toque en X, navegación `6 -> 5` y el inicio de la
transición específica de 300 ms. Una primera lectura serie se interrumpió
por un error de Windows en COM5; al reconectar se registró LVGL activo a
los 20 y 30 segundos con 288.519 bytes de heap interno libre. Esto valida
la ejecución de la ruta y las señales de actividad observadas; la fluidez
visual de la animación requiere confirmación física del usuario.

## Dissolve para calendario

Compilación de la política de páginas diferentes: 1.766.464 bytes
(`0x1af440`), 86 % libre en la partición, sin errores de compilación.
Se usa Dissolve de 300 ms con Ease Out tanto al entrar al calendario como
al abrir el detalle y regresar. La transición por componentes de
Today ↔ Agregar se conserva. La prueba física pendiente es
Today → calendario → día 11 → X → X, observando el fundido en cada paso.

## Bienvenida automática

Configuración solicitada: After delay 800 ms → Today, Smart Animate,
Ease In Out, 300 ms. Un temporizador LVGL de una sola ejecución arranca
al finalizar la entrada a Bienvenida. Se cancela si se abandona la vista
y no puede dispararse desde otra pantalla. El toque ya no adelanta la espera.
Los objetos de Today están preparados desde el arranque.
La compilación terminó sin errores: 1.767.104 bytes (`0x1af6c0`),
86 % libre en la partición. Se cargó en COM5 con hashes verificados y sin
escritura de NVS. La comprobación visual consiste en llegar a Bienvenida
y observar el paso automático a Today sin pulsarla.
