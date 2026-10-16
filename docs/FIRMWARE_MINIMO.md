# Fase 0–1: base de firmware mínimo

Fecha: 16 de octubre de 2026  
Rama: `havit-v2`  
ESP-IDF: 5.4.2  
Target: ESP32-S3, Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0

## Alcance implementado

- Arranque con ESP-IDF y FreeRTOS.
- Panel circular 360×360 mediante ST77916 y táctil CST816S.
- LVGL 9.5 con búferes RGB565 parciales de 16 líneas en RAM interna DMA.
- Tick de LVGL de 2 ms y tarea de manejo de 8 KiB.
- Vista negra sin pantallas ni recursos gráficos antiguos.
- Detección táctil mediante interrupción cuando el driver la habilita; el monitor serie imprime las coordenadas.
- PSRAM octal habilitada. LVGL en modo de configuración mínimo; ejemplos y demos desactivados.
- No se inicializa, escribe ni borra NVS desde la aplicación mínima.

## Estructura activa

- `core/`: arranque y ciclo de LVGL.
- `hardware/`: placa, controlador de pantalla y entrada táctil.
- `ui/components/`, `ui/animations/`, `ui/styles/`: directorios de extensión; no contienen componentes gráficos heredados.
- `ui/modules/`: vista negra actual.
- `services/`: vacío salvo su nota de alcance.
- `firmware/components/`: adaptadores ESP-IDF que integran los módulos anteriores en el sistema de componentes.
- `firmware/sdkconfig.defaults`: configuración reproducible del target, pantalla, táctil, PSRAM y LVGL.

## Respaldo anterior

El commit local `0af752b` en `main` conserva el firmware heredado, los XML y los recursos versionables. La carpeta hermana `lvgl_vscode_project_backup_pre_gui_refactor_20261015` permanece intacta. Los archivos generados de compilación y `managed_components` se excluyen y pueden regenerarse con ESP-IDF y `dependencies.lock`.

En `havit-v2` no están versionados `src/`, `ui/screens/`, XML, imágenes, fuentes exportadas, componentes gráficos generados, documentación del flujo previo, servicios Wi-Fi/BLE, autenticación, NVS store ni la lógica antigua de aplicación. Esa limpieza solo afecta esta rama; no borra el commit `main` ni la carpeta hermana. El UI de firmware compila únicamente `ui/modules/blank_screen.c`; LVGL y sus fuentes internas se obtienen como dependencia fijada en `dependencies.lock`.

## Compilación

Verificada con:

```powershell
idf.py -C firmware build
```

Resultado: compilación limpia correcta con `idf.py -C firmware clean` seguida de `idf.py -C firmware build`. La imagen de aplicación mide **518,016 bytes** (`0x7e780`); la partición de aplicación es de 12 MiB y conserva **96% libre**. El mapa de compilación contiene las unidades propias de pantalla, táctil, núcleo, pantalla negra y `main`; no contiene fuentes bajo `ui/components/`, `ui/screens/`, imágenes ni fuentes exportadas.

| Región estática | Uso | Libre | Total |
| --- | ---: | ---: | ---: |
| DIRAM | 77,251 B (22.6%) | 264,509 B | 341,760 B |
| IRAM | 16,383 B (99.99%) | 1 B | 16,384 B |
| RTC FAST | 52 B (0.63%) | 8,140 B | 8,192 B |

El uso de heap disponible en ejecución, incluida la PSRAM, requiere medirlo en el dispositivo. La ejecución y entrada táctil tampoco se han validado físicamente todavía.

## Prueba física pendiente

Conecta el ESP32-S3 por USB y sustituye `COMx` por el puerto serie del dispositivo:

```powershell
idf.py -C firmware -p COMx flash monitor
```

Al reiniciar, la pantalla debe quedar negra. Al tocarla, el monitor debe mostrar `Touch detected at x=… y=…`. No uses `erase-flash`: la aplicación no modifica NVS y la tabla mantiene su partición NVS.

Prueba física completada en `COM5`: el equipo arrancó con la pantalla negra, inicializó ST77916 y CST816S, y el monitor serie registró múltiples eventos `Touch detected` con coordenadas. No se borró la partición NVS.

La animación de ojos queda fuera de esta fase y se implementará solo tras la autorización para comenzar ese módulo.
