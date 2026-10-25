# Sesión vinculada y reposo de pantalla

## Después de vincular

- El resumen de éxito dice **Sesión vinculada** y ya no muestra el ID de unidad
  ni el `binding_id`.
- Al tocar **Continuar**, se abre la bienvenida y después el flujo automático
  lleva a Today.
- El nombre de bienvenida usa `displayName` o `accountName` si el endpoint
  autenticado `GET /v1/device/link` los incluye. El contrato actual que conoce
  el firmware solo garantiza `deviceId`, `state` y `bindingId`; por eso la
  alternativa visible es «¡Hola!» hasta que el backend envíe el nombre.
- El nombre de cuenta solo se mantiene en RAM y se limpia al desvincular o
  restablecer la sesión. No se guarda en NVS ni se expone en el endpoint local.

## Atenuación de pantalla

- Tras 30 segundos sin toque, el driver PWM de la retroiluminación baja al 20 %.
- Un toque durante la atenuación restaura el 100 % y se procesa normalmente.
- Si pasan otros 30 segundos sin actividad (60 segundos totales), se apaga la
  retroiluminación. El toque siguiente la reactiva al 100 % y despierta el
  panel sin ejecutar accidentalmente el control que estaba debajo.
- Wi-Fi/BLE, tareas y NVS permanecen activos; no se borra memoria persistente.

La validación final de brillo y reactivación requiere dejar el panel sin tocar
durante 30 y 60 segundos y comprobar la respuesta al toque en el ESP32.

## Resultado en la placa conectada

- Build ESP-IDF completado; binario de 4,346,496 bytes y 65 % libre en la
  partición de aplicación.
- Cargado en COM5; se verificaron los hashes de la aplicación, bootloader y
  tabla de particiones. No se usó `erase-flash`; NVS quedó fuera de los rangos
  escritos.
- El monitor serial registró **20 % al segundo 31** y **retroiluminación apagada
  al segundo 61**. Wi-Fi siguió activo.
- La reactivación mediante un toque físico queda pendiente de comprobar en la
  placa; abrir COM5 con el control USB reinició la placa durante la prueba, así
  que ese gesto no sirve para confirmar el despertar desde apagado.
