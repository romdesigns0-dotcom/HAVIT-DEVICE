# Renderizado y desplazamiento

El formulario conserva sus objetos LVGL y usa una caché RGB565 de 360×1091
durante esta etapa de prototipo estático. Se genera al preparar las vistas,
no al arrastrar. Los textos e iconos siguen siendo la fuente de la composición;
la caché debe regenerarse cuando cambien los datos. Si la reserva de caché
falla, el formulario mantiene el renderizado nativo de sus objetos.

## Presentación del fotograma

`hardware/display.cpp` usa dos buffers RGB565 completos en PSRAM, de 259,200
bytes cada uno. Conserva la invalidación por regiones para las animaciones
pequeñas. Durante el scroll, la región incluye el viewport entero y cabe en un
solo buffer; LVGL termina de componerla antes de enviarla. Una
tarea del segundo núcleo copia y transmite la imagen ya terminada mediante un
buffer DMA interno de 28,800 bytes (40 líneas). El límite del bus SPI es 32,768
bytes por transacción: estos segmentos son de transporte y no provocan que se
dibujen componentes entre envíos. Solo se llama `lv_display_flush_ready` al
terminar el fotograma completo.

El renderizado trabaja en el núcleo 0 y la transferencia en el núcleo 1. El
refresco tiene un periodo objetivo de 16 ms; no es una promesa de 60 FPS. Se
registran tiempos reales de preparación y transferencia en ventanas de 30
actualizaciones, junto al tamaño de la región. En el scroll cada región es el
viewport completo. No hay sincronización TE implementada; preparar un fotograma
completo por sí solo no garantiza eliminar todo tearing del panel.

La lectura de LVGL del táctil se programa cada 10 ms. El viewport recibe el
gesto y distingue `CLICKED` del arrastre para no accionar X o Generar al iniciar
un scroll sobre esos controles.

## Configuración y memoria

- CPU: 240 MHz; PSRAM octal: 80 MHz. Son configuraciones de ESP-IDF para este
  hardware. La comprobación de memoria de arranque debe pasar después de la
  carga. Las frecuencias aumentan el consumo activo frente a 160/40 MHz;
  corriente y autonomía no se han medido.
- Buffers completos: 518,400 bytes de PSRAM.
- Caché del formulario: 785,520 bytes más las estructuras de LVGL.
- Buffer de transporte: 28,800 bytes internos DMA, más la cola y la tarea.
- Los iconos de edición, voz, reloj, X y separadores se comparten con otros
  módulos después de comparar sus píxeles y dimensiones.
- El centro de Default 4 es transparente en el recurso de ejecución para
  conservar negro y saturación. Su SVG de Figma permanece sin modificar.

## Validación

### Corrección del bloqueo en ojos, 22/10/2026

El monitor de COM5 mostró watchdog de IDLE0 con `havit_lvgl` dentro de
`wait_for_flushing` (`lv_refr.c`). El envío se realiza desde otra tarea y LVGL
está configurado con `CONFIG_LV_OS_NONE`. Se sustituye la espera activa por
`lv_display_set_flush_wait_cb`: un semáforo de FreeRTOS señala el final de la
transferencia y la tarea LVGL libera su propio estado de flushing. Se conserva
el doble buffer y el renderizado parcial. La tabla instalada se leyó por USB y
coincide con la local; la corrección solo requiere escribir la aplicación en
`0x10000`, sin reescribir tabla, identidad ni NVS.

Compilación incremental ESP-IDF 5.4.2 correcta: 4,241,744 bytes, 66 % de
partición libre. Carga en COM5 completada con hash verificado. El monitor serie
registró entrada a Login tras la intro, eventos táctiles y mensajes `LVGL alive`
a los 11, 21 y 31 segundos, sin watchdog durante los 35 segundos observados.
La revisión visual final de la animación corresponde a la prueba del usuario.

- ESP-IDF 5.4.2 / LVGL 9.5: compilación sin errores. El fallo interno de GCC en
  `esp_lcd_panel_rgb.c` se resolvió recompilando con Ninja `-j 1`, sin editar
  ESP-IDF. Firmware: 3,026,848 bytes; 76 % de partición libre.
- Carga a COM5 con hashes verificados, sin borrar NVS. Arranque a 240 MHz,
  configuración de PSRAM de 80 MHz y prueba de memoria correcta.
- Caché confirmada en el dispositivo: 360×1091, 785,520 bytes; pasó la prueba
  del contenido situado por debajo del viewport. Rango táctil: 731 px.
- A los 31 segundos: 244,071 bytes internos libres y 6,812,592 bytes de PSRAM
  libres. No hubo reinicios en el recorrido registrado.
- Ojos: ventanas de actualización con unos 8.6–9.4 ms de preparación y
  1.5 ms de transferencia. Las ventanas que incluyeron formulario completo y
  transiciones registraron unos 79–85 ms de preparación y 19–23 ms de envío;
  mezclan regiones y estados, por lo que no representan un contador de FPS.
- Se registró desplazamiento hasta y=731, activación de Generar, generación,
  línea de tiempo, Desglose, Enviar y retorno a Today.
- El usuario confirmó: «Sí, ambos mejoraron» al evaluar contraste y scroll.
  La fluidez percibida está validada con esa prueba; no se afirma 60 FPS.
- Pasó la comprobación de recursos: centro transparente, exterior opaco y
  los 13 recursos de Teams referenciados con dimensiones nativas.
