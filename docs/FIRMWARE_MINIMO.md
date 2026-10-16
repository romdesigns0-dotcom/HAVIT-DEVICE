# Firmware HAVIT — introducción y primer paso de acceso

Fecha: 16 de octubre de 2026  
Rama: `havit-v2`  
ESP-IDF: 5.4.2  
Target: ESP32-S3, Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0

## Estado de la etapa

El arranque muestra una sola vista LVGL de ojos sobre el fondo compartido de `PANTALLA DEFAULT`. El usuario validó físicamente el aspecto del aro y de los ojos. La vista mantiene los parpadeos en loop.

Al tocar cualquier punto de la vista de ojos, la transición queda solicitada y espera a completar el siguiente pestañeo antes de pasar a `LOG IN 1`. Esta regla también cubre un toque recibido antes del primer pestañeo.

`LOG IN 1` se basa en el frame Figma `349:2084` y muestra “Hola, Soy Havit.”, “Empecemos por aquí” y el círculo azul con flecha. Tras el pestañeo solicitado, los ojos se desvanecen con Ease in/out durante 300 ms; título, subtítulo y botón aparecen después en secuencia con opacidad escalonada. Todo ocurre dentro de la misma vista LVGL, sin reconstruir otra pantalla. El botón queda visual, pendiente de conectar al próximo paso hasta que estén listos los detalles de navegación.

Los frames `LOG IN 2`, `LOG IN WIFI`, `LOG IN PIN` y `Welcome Back` están localizados en la sección `PANTALLA 2` del Figma; esperan los elementos que el usuario agregará. No se implementan todavía.

Las cuatro poses Figma de los ojos son estáticas, sin cronología asociada. Los tiempos escogidos para la animación LVGL son apertura 460 ms, ajuste 180 ms y parpadeo cada 3.4 s; el usuario validó visualmente el resultado.

## Estructura activa

- `core/`: arranque y ciclo LVGL.
- `hardware/`: placa, pantalla ST77916 y táctil CST816S.
- `ui/components/havit_default_background.c`: fondo negro compartido con aro degradado.
- `ui/include/havit_default_background.h`: API del fondo común.
- `ui/assets/havit_default_background.svg`: aro vectorial de `PANTALLA DEFAULT`.
- `ui/modules/eyes_intro.c`: vista de ojos y detección de toque.
- `ui/animations/eyes_intro_animation.c`: apertura, parpadeo en loop y callback de salida tras un pestañeo completo.
- `ui/modules/login1.c`: capas de texto e imágenes LVGL de la primera pantalla de acceso.
- `ui/assets/login1_continue_circle.svg`, `login1_continue_arrow.svg`: recursos SVG locales del botón de `LOG IN 1`.
- `ui/fonts/InstrumentSans-Regular.ttf`, `OFL.txt` y `instrument_sans_regular_14.c` / `instrument_sans_regular_22.c`: fuente Instrument Sans con licencia SIL OFL y fuentes LVGL generadas sin compresión de glifos para los tamaños usados. LVGL tiene desactivado el decodificador de fuentes comprimidas.
- `services/`: sin servicios de aplicación añadidos todavía.
- `firmware/components/`: adaptadores ESP-IDF y configuración de dependencias.

El proyecto conserva ESP-IDF 5.4.2, FreeRTOS, LVGL 9.5, pantalla circular 360×360, táctil, PSRAM octal de 8 MB y búferes parciales RGB565 de 16 líneas en DMA interno. LVGL tiene habilitados SVG/ThorVG, imágenes y labels. Los textos usan Instrument Sans 22 px y 14 px, integradas como fuentes LVGL estáticas. La aplicación no inicializa, escribe ni borra NVS.

## Compilación

`idf.py -C firmware build` terminó correctamente con las fuentes regeneradas y la transición a `LOG IN 1`. El binario generado es `firmware/build/havit_os_firmware.bin`, tamaño **727,248 bytes** (`0xB18D0`); `idf.py size` reporta **727,128 bytes** en la imagen. Queda **94% libre** en la partición de aplicación de 12 MiB.

| Región estática | Uso | Libre | Total |
| --- | ---: | ---: | ---: |
| DIRAM | 78,235 B | 263,525 B | 341,760 B |
| IRAM | 16,383 B | 1 B | 16,384 B |
| RTC FAST | 52 B | 8,140 B | 8,192 B |

El heap libre y el uso dinámico de PSRAM se medirán en ejecución.

La compilación propia incluye `havit_default_background.c`, `eyes_intro.c`, `login1.c` y `eyes_intro_animation.c`. No incluye pantallas antiguas, XML históricos, recursos gráficos viejos, navegación anterior, Wi‑Fi/BLE ni autenticación.

## Carga y prueba física de esta etapa

La compilación corregida con Instrument Sans sin compresión y la transición a `LOG IN 1` se cargó por COM5; esptool verificó los hashes. No se ejecutó `erase-flash`; se escribieron bootloader, aplicación y tabla de particiones, dejando intacta NVS. El monitor confirmó el arranque de ESP-IDF, PSRAM, ST77916, CST816S y la vista de ojos sin panic. Falta la confirmación visual física de que los glifos y la secuencia de Login 1 ya se vean correctamente.

Inspecciona ahora físicamente la nueva compilación:

```powershell
idf.py -C firmware -p COM5 -b 460800 flash
idf.py -C firmware -p COM5 monitor
```

Al arrancar, comprueba que los ojos sigan parpadeando. Toca cualquier punto: debe terminar un parpadeo completo, desvanecer los ojos en 300 ms y revelar por capas el título, subtítulo y botón; verifica que la tipografía sea Instrument Sans y que el botón no aparezca como rectángulos. Sal del monitor con `Ctrl+]` para liberar COM5. No uses `erase-flash`: la aplicación no borra ni modifica NVS.

La versión anterior de ojos y fondo compartido ya fue validada visualmente por el usuario. Esta etapa queda detenida para validar la transición y `LOG IN 1` en el dispositivo antes de continuar.
## Corrección de caracteres en LOG IN 1

Se habilitó explícitamente `CONFIG_LV_TXT_ENC_UTF8=y` en `firmware/sdkconfig.defaults`. Con `CONFIG_LV_CONF_MINIMAL`, LVGL estaba seleccionando ASCII por defecto; las cadenas UTF-8 (por ejemplo, «aquí») se interpretaban con glifos incorrectos. La configuración local resultante confirma UTF-8 activo y ASCII desactivado.

La compilación terminó correctamente: `firmware/build/havit_os_firmware.bin`, **727,744 bytes** (`0xB1AC0`), con 94% de espacio libre en la partición de aplicación (12 MiB). Se cargó por COM5; esptool verificó los hashes. No se usó `erase-flash` y la partición NVS (`0x9000`) no fue escrita.

El monitor serie confirmó el arranque de la aplicación, PSRAM, pantalla ST77916, táctil CST816S y la vista de ojos sin panic. Falta la comprobación visual del texto en el dispositivo. En LOG IN 1, confirma que se lea «Hola, Soy Havit.» y «Empecemos por aquí», especialmente la `í` acentuada.
