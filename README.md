# HAVIT firmware V2

Firmware modular para ESP32-S3 y pantalla circular de 360×360.

## Estado

La base actual inicializa la placa Waveshare ESP32-S3-Touch-LCD-1.85 con pantalla ST77916 y táctil CST816S, LVGL 9.5, renderizado parcial RGB565 con buffers DMA y una vista negra que registra eventos táctiles.

Las pantallas y recursos de la versión anterior se conservaron en el commit `main` (`0af752b`, respaldo del firmware heredado) y en la carpeta hermana `lvgl_vscode_project_backup_pre_gui_refactor_20261015`. Esta rama no los compila ni contiene sus XML.

## Estructura

- `core/`: arranque y ciclo de LVGL.
- `hardware/`: placa, pantalla y táctil.
- `ui/components/`: componentes compartidos (vacío hasta necesitarlos).
- `ui/modules/`: vistas funcionales; actualmente solo pantalla negra.
- `ui/animations/`: animaciones (vacío hasta necesitarlas).
- `ui/styles/`: estilos y recursos visuales compartidos (vacío por ahora).
- `services/`: reservado para servicios que se incorporarán cuando hagan falta.
- `firmware/`: proyecto ESP-IDF, configuración y particiones.

## Compilar

Con ESP-IDF 5.4.2 activado:

```powershell
idf.py -C firmware build
```

Para probar en el ESP32-S3 conectado:

```powershell
idf.py -C firmware flash monitor
```

La pantalla debe permanecer negra. Un toque debe generar `Touch detected at x=… y=…` en el monitor serie. No se escribe ni borra NVS en esta fase.
