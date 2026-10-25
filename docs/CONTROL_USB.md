# Control táctil desde la computadora

Implementación de desarrollo: `CONFIG_HAVIT_DEBUG_USB_CONTROL=y`. Requiere consola
USB Serial/JTAG. Desactivar esa opción en producción.

El ESP32 no dispone de un MCP instalado. `tools/usb_control.py` permite al asistente
enviar comandos a la placa por el mismo puerto COM usado para los logs. Cerrar
otros monitores del puerto antes de usarlo. No usar simultáneamente dos lectores.

Desde este proyecto, con el Python de ESP-IDF que contiene pyserial:

```powershell
python tools/usb_control.py --port COM5 status
python tools/usb_control.py --port COM5 tap 180 180
python tools/usb_control.py --port COM5 swipe 180 270 180 100 600
```

Protocolo textual, terminado en LF:

```text
havit status
havit tap 180 180
havit swipe 180 270 180 100 600
```

Coordenadas nativas 0..359. Duración de swipe 50..5000 ms. Un toque mantiene
presión 100 ms. Se entrega una liberación previa de 200 ms y otra posterior de
200 ms. Solo se acepta un gesto a la vez. Los comandos se procesan en la tarea
LVGL y generan entrada PRESSED/RELEASED en el mismo lector que usa CST816S;
se ejecutan tanto el callback de navegación como los eventos LVGL, incluido scroll.
Durante el gesto se ignora el táctil físico para evitar mezclar dos punteros.

`USB command ESP_OK` confirma aceptación, no la navegación. `USB gesture finished`
confirma fin de la secuencia; consultar `status` después para comprobar el resultado.
`screen` corresponde al enum `havit_ui_screen_t` de `ui/include/havit_ui.h`.
No hay captura de pantalla remota implementada: status y logs no comprueban
visualmente el renderizado ni sustituyen una prueba del sensor físico.

El canal no permite leer tokens, PoP, contraseñas ni escribir NVS directamente.
Los toques sí ejecutan las acciones normales de la interfaz: no pulsar una
confirmación de borrado/restablecimiento sin intención de realizarla.

Fundamento: [entrada de puntero LVGL](https://lvgl.io/docs/open/9.4/details/main-modules/indev/overview)
y [consola USB ESP-IDF 5.4.2](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/api-guides/stdio.html).

## Validación física — 2026-10-22

- ESP-IDF 5.4.2 compiló con la opción habilitada. Binario `0x40bf50`
  (4,243,280 bytes), 66 % libre. Se escribió solo aplicación en `0x10000`,
  hash verificado por esptool. No se escribió tabla, bootloader ni identidad.
- `status` respondió en COM5: `screen=0 network=1 identity=1 wifi=0 linked=0`.
- `tap 180 180`: gesto confirmado; salió de los ojos hacia Login 1.
- `tap 180 238`: gesto confirmado y posterior `status screen=1` (Login 2).
- `tap 180 245`: gesto confirmado y posterior `status screen=4` (BLE ready).
- La placa quedó en configuración BLE, con identidad cargada. No se ejecutó
  restablecimiento de Wi-Fi ni se enviaron contraseñas durante esta validación.
- El cliente rechazó `tap 360 180` antes de abrir el puerto.
- No se validó físicamente scroll con swipe ni se obtuvo captura visual del ESP32.
