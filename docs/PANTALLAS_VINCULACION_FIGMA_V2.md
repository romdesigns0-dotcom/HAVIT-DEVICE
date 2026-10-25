# Pantallas V2 de vinculación

## Referencia

Los siete frames consultados en la página `1X1` del archivo HAVIT son:

- `572:2` — Bluetooth disponible.
- `576:27` — Conexión Bluetooth segura.
- `576:33` — Buscando redes Wi-Fi.
- `572:10` — Conectando a Wi-Fi.
- `572:18` — Confirmar con toque.
- `573:15` — Confirmar con PIN.
- `572:24` — Dispositivo vinculado / sincronizando.

## Apariencia aplicada

- Fondo negro y aro Default 2 compartidos; las siete vistas son transparentes
  para dejar visible el fondo común.
- Encabezado centrado en y=70, Instrument Sans 22, blanco al 86 %.
- Estado en y=104, Instrument Sans 15, blanco al 55 %.
- Contenido en y=157, ancho 296 px, Instrument Sans 21, blanco al 90 %.
- Los frames con acción llevan una cápsula `#73979A` de 200×48 px en (80,270),
  con texto Instrument Sans 17. Confirmar con toque y Confirmar con PIN no tienen
  botón, tal como en Figma.
- Confirmar con toque acepta el toque de la pantalla y solicita actualizar el
  estado de vinculación. El PIN sigue siendo dinámico.

El gris `#828282` que aparece alrededor de los frames en el lienzo de Figma es
el fondo del canvas, no el fondo de pantalla del dispositivo.

La implementación está en `ui/modules/device_setup.c`; el manejo de toque está
en `ui/modules/login_flow.c`. Las fuentes Instrument Sans 17 y 21 se generan
con `tools/generate_today_fonts.ps1`.

## Validación

- ESP-IDF 5.4.2 / ESP32-S3: compilación correcta.
- Firmware: 4,404,320 bytes (`0x433460`); 65 % libre en la partición de app de
  12 MiB.
- Cargado en COM5; esptool verificó hashes. No se usó `erase-flash` y NVS en
  `0x9000` no se escribió.
- Revisión actual: el fondo gris del lienzo se descartó; las vistas usan el aro
  negro compartido como solicitaste.
- Pendiente: confirmar visualmente en la pantalla que el gris, la tipografía y
  la ubicación de los controles coincidan con Figma.
