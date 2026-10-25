# HAVIT — firmware para ESP32-S3

HAVIT es el firmware de un asistente personal con pantalla táctil circular de
360 × 360 px. Se desarrolla con ESP-IDF, FreeRTOS y LVGL 9.5 en C/C++ para una
placa Waveshare ESP32-S3-Touch-LCD-1.85, con pantalla ST77916, táctil CST816S y
PSRAM.

La interfaz se diseña primero en [Figma](https://www.figma.com/design/3bmX64I7Rt85DV3boqOlsc/Havit-Device-UI). Figma define la apariencia y el comportamiento esperado; el firmware recrea esas vistas con objetos nativos LVGL para que el texto siga siendo texto, los controles respondan al toque y las animaciones se ejecuten en el dispositivo.

## Cómo pasamos de Figma al ESP32

1. **Elegir el flujo y sus frames.** Se revisan en Figma solo las pantallas de la
   funcionalidad que toca implementar. No se exporta todo el archivo. Se anotan
   sus IDs, dimensiones, posiciones, colores, textos, tipografías e interacciones.
2. **Separar lo fijo de lo interactivo.** Fondos, títulos, listas, botones,
   indicadores y estados se identifican como objetos reutilizables, controles
   táctiles, recursos gráficos o contenido dinámico.
3. **Recrear la vista en LVGL.** Cada módulo se implementa en C dentro de
   `ui/modules/`. Los textos se dibujan con etiquetas LVGL; los botones y las
   listas usan objetos que pueden recibir eventos. La lógica de navegación y
   estados vive separada de la composición visual.
4. **Convertir solo los recursos necesarios.** SVG e iconos se guardan en el
   repositorio y scripts locales los convierten a descriptores LVGL incluidos en
   el firmware. Instrument Sans se convierte a fuentes C con los tamaños y
   glifos requeridos. No se usa una captura de pantalla como interfaz final.
5. **Traducir las interacciones.** Las transiciones de Figma se implementan con
   animaciones LVGL de opacidad, posición, tamaño o estado. Si varios frames son
   poses de una animación, se conservan como estados dentro de una sola vista;
   no se crean pantallas distintas sin necesidad.
6. **Compilar y probar en la placa.** Cada entrega se compila, se verifica en el
   ESP32 y se ajusta con una prueba física antes de empezar el siguiente módulo.

El canvas gris de Figma puede ser solo el área de trabajo. El fondo previsto para
la interfaz HAVIT es negro con el aro de color compartido; se confirma por el
frame y la referencia del dispositivo antes de cambiarlo.

## Qué incluye actualmente

- Arranque de ESP-IDF y FreeRTOS, configuración reproducible para ESP32-S3 y
  dependencias fijadas en `firmware/dependencies.lock`.
- Inicialización de ST77916 y CST816S, buffers de pantalla y renderizado LVGL
  con PSRAM.
- Introducción animada de los ojos, flujo de acceso y pantallas de configuración
  BLE/Wi-Fi y vinculación.
- Módulos visuales de Today, calendario, tareas del día, tarea rápida y flujo de
  tarea en equipo. Son una base de desarrollo que se valida por etapas en el
  hardware.
- Servicios de conectividad, aprovisionamiento BLE seguro, Wi-Fi, vinculación
  con la cuenta y estado de tareas.
- Atenuación y apagado de pantalla por inactividad, con reactivación táctil.

El detalle de cada flujo y sus pruebas está en [`docs/`](docs/). Los XML y las
pantallas heredadas no forman parte del binario V2.

## Organización del código

```text
core/                         Ciclo principal de la aplicación y FreeRTOS
firmware/                     Proyecto ESP-IDF, configuración y particiones
hardware/                     Placa, pantalla, táctil y control de brillo
services/                     BLE, Wi-Fi, vinculación y servicios de datos
ui/components/                Fondos y componentes visuales compartidos
ui/modules/                   Vistas y flujos funcionales independientes
ui/animations/                Animaciones y transiciones LVGL
ui/styles/                    Tokens visuales y guías de estilo
ui/assets/                    SVG originales y descriptores LVGL generados
ui/fonts/                     Fuentes Instrument Sans en formato LVGL C
tools/                        Generadores y herramientas locales de desarrollo
docs/                         Especificaciones, decisiones y validaciones
```

### Qué hace cada capa

- **`core/`** inicia el firmware, mantiene el ciclo de LVGL y coordina el estado
  de la aplicación.
- **`hardware/`** aísla el perfil de la placa y los controladores de display,
  táctil y retroiluminación.
- **`services/`** maneja conectividad y datos sin dibujar pantallas. Las vistas
  consultan estos servicios y presentan sus estados.
- **`ui/modules/`** compone cada flujo. Los módulos pueden cambiar sin mezclar
  la inicialización de pantalla o la lógica de red con el diseño.
- **`ui/components/`, `ui/styles/` y `ui/animations/`** reúnen recursos y
  comportamientos visuales que comparten varias vistas.
- **`firmware/`** conecta esas capas como componentes de ESP-IDF y define el
  target, las opciones de LVGL, las dependencias y la tabla de particiones.

## Compilar y cargar

Requisitos: ESP-IDF **5.4.2**, Python del entorno de ESP-IDF y target
`esp32s3`. En PowerShell, con ESP-IDF activado:

```powershell
idf.py -C firmware build
idf.py -C firmware -p COM5 flash
idf.py -C firmware -p COM5 monitor
```

Sustituye `COM5` por el puerto de la placa conectada. Al compilar, ESP-IDF
descarga los componentes fijados en `dependencies.lock`; estos quedan en
`firmware/managed_components/`, que no se versiona.

Después de cargar, comprueba el arranque, la pantalla, el toque, las
interacciones de la vista y el uso de memoria. Las métricas de cada compilación
se guardan con la documentación de la etapa correspondiente.

**No uses `erase-flash` como procedimiento normal de actualización.** La
identidad, la vinculación y la configuración de red viven en particiones NVS.
Un cambio de firmware no debe borrarlas automáticamente.

## Convenciones para nuevas pantallas

- Trabajar un flujo o módulo a la vez y enlazar los frames concretos de Figma en
  su documentación.
- Mantener textos editables con las fuentes LVGL; usar imágenes solo para
  ilustraciones e iconos que lo necesiten.
- Guardar los SVG fuente y los scripts de conversión para que los recursos se
  puedan regenerar.
- Reutilizar el fondo y componentes compartidos sin cubrirlos con rectángulos
  opacos accidentales.
- Compilar y probar cada etapa en el dispositivo antes de continuar.
- No subir contraseñas, tokens, claves, `sdkconfig` local, credenciales ni
  archivos de compilación. Usa aprovisionamiento seguro para las credenciales
  específicas de cada unidad.

## Documentación útil

- [Introducción y login](docs/CORRECCION_TOUCH_LOGIN.md)
- [Vinculación de dispositivo](docs/VINCULACION_DISPOSITIVO.md)
- [Flujo BLE y Wi-Fi](docs/BLE_WIFI_IOS.md)
- [Today y calendario](docs/TODAY_CALENDARIO.md)
- [Tarea rápida](docs/TAREA_RAPIDA.md)
- [Tarea en equipo](docs/FLUJO_EQUIPOS.md)
- [Desplazamiento y renderizado](docs/RENDERIZADO_SCROLL.md)
- [Fondos de pantalla](docs/FONDOS_DEFAULT.md)
