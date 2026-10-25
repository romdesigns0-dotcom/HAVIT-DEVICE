# Botón central de Today: tareas del día

## Fuente Figma

- `838:269` — lista de tareas de Hoy actualizada.
- `838:316` — modo de selección actualizado.
- `826:522` — detalle de ACT 8 - Fuentes.
- Cambios de lista/selección/detalle: Smart Animate, Ease Out, 300 ms.

Los SVG usados por estas vistas se conservan en `ui/assets/today_tasks/` y se
convierten a descriptores LVGL con `py -3.13 tools/generate_today_task_assets.py`.
La lista y el modo de selección se implementan en
`ui/modules/today_task_screens.c`. Ambos reflejan ahora las tres tareas de los
frames nuevos, el chevrón del selector de fecha, los iconos de navegación y las
casillas de selección redimensionadas. El modo de selección coloca la papelera
rosa a la izquierda y la acción blanca a la derecha.
La navegación global se mantiene en `ui/modules/login_flow.c`.

## Interacciones incluidas

- Centro de Today abre la lista de tareas.
- Cerrar la lista vuelve a Today.
- Lápiz alterna lista y selección.
- La primera flecha abre el detalle de ACT 8; la X vuelve a la lista.
- Casillas alternan selección. El icono de papelera oculta las filas elegidas
  solo en esta vista demostrativa; no borra tareas del servicio ni de NVS.
- Empezar muestra el estado local “EN CURSO”; Terminado vuelve a la lista.
- El icono de compartir está dibujado, pero no se conectó a un servicio de
  compartir porque el diseño Figma no define esa acción.

## Validación de esta entrega

- Compilación ESP-IDF 5.4.2 exitosa en `build-today-tasks-3`.
- `havit_os_firmware.bin`: 4,343,600 bytes; 65 % libre en la partición de
  aplicación de 12 MiB.
- Cargado en COM5, ESP32-S3 con MAC terminada en `95:A8`; esptool verificó los
  hashes de bootloader, aplicación y tabla de particiones.
- No se ejecutó `erase-flash`; la escritura llegó a las particiones de
  bootloader, aplicación y tabla de particiones. NVS inicia en `0x9000` y no
  estuvo dentro de los rangos escritos.
- El monitor confirmó arranque, inicialización de ST77916 y CST816S y creación
  de las vistas de tareas. La respuesta táctil visual queda pendiente de tu
  prueba física.

## Prueba física

En Today, abre las tareas. Confirma que aparecen solo tres filas, que el botón
X vuelve a Today, que el lápiz alterna lista/selección y que la primera flecha
abre ACT 8. En selección, verifica las casillas y que papelera rosa y acción
blanca quedaron en el orden de Figma. La respuesta visual final debe validarse
en el panel físico antes de continuar con el siguiente grupo de pantallas.
