# Tarea Rápida

Referencia Figma: archivo `3bmX64I7Rt85DV3boqOlsc`, página `1X1`.
Frames `464:314` (formulario), `464:342` (temporizador) y `464:357`
(completado), en un lienzo de 360 × 360 px.

## Flujo implementado

- Agregar → burbuja derecha abre Tarea Rápida con Smart Animate, Ease Out,
  300 ms.
- El campo de nombre es editable con el teclado LVGL. Si se deja vacío, el
  temporizador muestra “Humanidades”, como en la referencia de Figma.
- Los selectores de minutos y segundos cambian la duración. El valor inicial
  es 00:00; se debe escoger una duración antes de iniciar.
- Cada vez que se abre el formulario de Tarea Rápida, ambos selectores se
  restablecen a `00`; no se conserva la selección de una visita anterior.
- EMPEZAR abre el temporizador y la marca blanca abre Completado; ambas
  transiciones usan Smart Animate, Ease Out, 300 ms. Al iniciar, comienza la
  cuenta regresiva.
- La marca blanca termina la tarea inmediatamente. Al llegar a cero también
  se abre Completado. La X cancela y vuelve al formulario.
- La X del formulario regresa a Agregar.
- La X también cierra el teclado si estaba abierto y luego regresa a Agregar.
- Al terminar ese retorno, se restauran explícitamente todos los componentes
  de Agregar para evitar mostrar solo el fondo compartido.
- El regreso Quick Task → Agregar usa la transición de componentes y no Dissolve,
  porque la salida debe restaurar las capas que se ocultaron al entrar.
- En Completado, cualquier toque de pantalla, incluida la burbuja verde de
  confirmación, regresa a Today.
- El icono de micrófono es solo visual; HAVIT todavía no tiene un servicio de
  reconocimiento de voz conectado.

El formulario se actualizó desde el frame `464:314`: título centrado en una
sola línea; campo editable con `NOMBRE` como placeholder visible cuando está
vacío (se reemplaza al escribir); línea en y=192; iconos de edición y voz en
(255, 148) y (281, 148). Los rodillos de 75 × 35 px están en (109, 220) y
(201, 220), con el texto de ambas partes alineado al centro y valores iniciales
`00:00`. Reloj y separador vectorial siguen las nuevas posiciones de Figma. La
X verde, línea, micrófono, lápiz, reloj y separador usan los SVG actuales
rasterizados para LVGL. Los otros estados siguen los frames `464:342` y
`464:357`.

Si el usuario intenta iniciar con `00:00`, la acción se detiene y se registra
que debe elegir una duración, evitando que el temporizador arranque con un
valor distinto al mostrado.

Se usan objetos LVGL y la tipografía Instrument Sans. Teclado, textarea,
button matrix, line y roller quedan habilitados en `sdkconfig.defaults` para
que la función compile desde una configuración limpia.

## Validación

- Corrección del 18/10/2026: cada navegación hacia el formulario restablece
  minutos y segundos al primer valor (`00`) con `LV_ANIM_OFF`.
- Compilación ESP-IDF 5.4.2 correcta: binario de 1,955,472 bytes, con 84 % libre
  en la partición de aplicación. Carga a COM5 correcta y hashes verificados;
  no se borró ni modificó NVS. Falta comprobar físicamente la reapertura del
  formulario tras cambiar la duración.
- Corrección del 18/10/2026: al abrir Quick Task se dejaban los hijos de Add en
  opacidad cero. Si se completaba la tarea, se volvía a Today y luego se abría
  Add con +, la entrada de disolución no restauraba las capas fijas porque
  solo animaba fecha, controles y círculos. La transición ahora restaura la
  opacidad de todos los hijos de la vista destino antes de animar.
- Compilación ESP-IDF 5.4.2 correcta: binario de 1,955,344 bytes, con 84 % libre
  en la partición de aplicación. Carga a COM5 correcta con hash verificado; no
  se borró ni modificó NVS.
- Corrección de legibilidad del 18/10/2026: título centrado en una sola línea;
  texto de los rodillos centrado explícitamente en sus partes normal y
  seleccionada; placeholder `NOMBRE` confirmado como guía del campo vacío.
- Compilación ESP-IDF 5.4.2 correcta: `havit_os_firmware.bin` de 1,955,232
  bytes; quedan 84 % libres en la partición de aplicación.
- Carga a COM5 correcta y hashes verificados. El monitor serial confirmó el
  arranque, PSRAM de 8 MB, inicialización ST77916 y CST816S, y la creación de
  las vistas. No se ejecutó `erase-flash` ni se modificó NVS.
- Compilación con ESP-IDF 5.4.2 tras actualizar el frame 11: correcta.
- `havit_os_firmware.bin`: 1,955,216 bytes; 84 % libre en la partición de
  aplicación de 12 MiB.
- Carga por COM5 correcta; esptool verificó los hashes y reinició el ESP32-S3.
- Monitor serial confirmó arranque, inicialización de pantalla ST77916 y
  táctil CST816S, y preparación de las vistas de Today y Tarea Rápida.
- La actualización escribió bootloader, aplicación y tabla de particiones. No
  ejecutó `erase-flash` ni escribió o borró la partición NVS.
- La revisión de la ruta táctil encontró y corrigió un bloqueo: la X ahora se
  procesa antes del teclado visible. El monitor serial está disponible para
  confirmar el toque físico de EMPEZAR y la X.
- La carga escribió bootloader, aplicación y tabla de particiones. No ejecutó
  `erase-flash` y no escribió ni borró la partición NVS.

## Prueba física pendiente

Desde Agregar, toca la burbuja derecha. En Tarea Rápida prueba editar el nombre,
cambiar minutos o segundos, iniciar la cuenta y cancelar con X. Luego toca la
marca blanca o espera el final automático. En Completado, toca distintos puntos
de la pantalla y la burbuja verde: ambos deben regresar a Today. Confirma en el
panel físico tamaños, legibilidad y respuesta táctil.
