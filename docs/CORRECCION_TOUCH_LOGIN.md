# Corrección de tacto y navegación, 16 de octubre de 2026

El monitor de la versión previa mostró reinicios `LoadProhibited` en
`lv_indev_read`, al acceder al display. Por tanto, el arranque inicial no
demostraba estabilidad. No se atribuye el fallo al controlador físico CST816S.

Se aisló la ruta de dibujo SVG: el aro se rasteriza una vez como RGB565 en
PSRAM (259.200 bytes), manteniendo centro (180,180), radio 171, grosor 8 y
degradado de #77B7DB a #699768. El botón circular es un objeto nativo LVGL.
El icono Google se convirtió del SVG local a ARGB8888 de 35×36; su conversión
se reproduce con `python tools/generate_google_icon.py`. Los SVG originales
permanecen en el proyecto como fuentes de referencia y no se embeben.

Con esta ruta de dibujo, la primera carga observada alcanzó 100 segundos
sin reinicios, con heap interno estable en 311.927 bytes y comprobaciones
de integridad satisfactorias. El monitor registró toque en (234,233),
salida de ojos, final de parpadeo, aparición de Login 1 y toque de la flecha.
Esto aísla el problema a la ruta gráfica anterior; no identifica todavía
la instrucción exacta que corrompía la memoria.

El driver notifica el primer toque de cada pulsación mediante un callback
ejecutado en el ciclo LVGL. La UI pide el parpadeo final o procesa las
coordenadas del botón. Las pulsaciones durante las transiciones se ignoran.
Login 1 permanece oculto hasta finalizar el desvanecimiento de los ojos.

## Recorrido visual

1. Tocar los ojos: terminar un parpadeo y revelar Login 1.
2. Pulsar la flecha: mostrar Login 2.
3. Pulsar Google: mostrar Configuración de Wifi / Buscando red.
4. Tocar la vista Wi-Fi: mostrar Vinculación / Mostrar Pin.
5. Tocar la vista PIN: mostrar Bienvenido de nuevo / Rom!

Los pasos Wi-Fi y PIN son navegación de demostración. No hay conexión Wi-Fi,
autenticación Google ni generación de PIN implementadas. Estas dos vistas
no tienen botones en la referencia visual suministrada. Bienvenida termina
el recorrido. Las transiciones duran 300 ms con ease in/out.

La carga usa `idf.py -C firmware -p COM5 flash`, sin `erase-flash` y sin
escribir NVS. Se conservan logs de coordenadas, transiciones y una señal de
actividad cada 10 segundos con comprobación del heap para la prueba física.

La compilación final con los cinco estados y el botón Google terminó sin
errores: binario de 854.448 bytes (`0xD09B0`), 93% libre en la partición.
Esptool verificó los hashes al cargarla en COM5. El usuario confirmó que
el recorrido llega a Bienvenida. El monitor registró la salida de ojos
a los 18,7 s y las cuatro transiciones `0 -> 1 -> 2 -> 3 -> 4` entre los
20,3 y 22,7 s, sin reinicios. El heap interno se mantuvo en 310.375 bytes
en los controles de 10 y 20 segundos, con integridad satisfactoria.
Esta validación cubre el tacto y la navegación visual, no autenticación
ni conectividad. Se cerró el monitor al terminar para liberar COM5.

## Flecha de continuar

Se sustituyó `LV_SYMBOL_RIGHT` por una imagen ARGB8888 de 29×17 basada
en `login1_continue_arrow.svg`: línea horizontal, punta abierta, trazo de
3 px y extremos redondeados. Se centra en el círculo azul de 54×54.
El recurso se reproduce con `python tools/generate_continue_arrow.py`.
Se mantiene el mismo destino y área táctil.
La compilación de la flecha terminó correctamente: 830.928 bytes
(`0xCADD0`). Se cargó en COM5 con hashes
verificados y sin escritura de NVS. El monitor confirmó la inicialización
de pantalla, táctil y vista de ojos; el aspecto de la flecha queda pendiente
de confirmación visual del usuario.
En esta carga el monitor volvió a registrar las cuatro transiciones hasta
Bienvenida, sin reinicios y con 310.251 bytes de heap interno libre.
