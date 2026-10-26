# Vinculación de HAVIT

Este documento describe el flujo vigente entre la app iOS, la placa ESP32-S3 y
el backend Firebase. Los identificadores de dispositivos reales y sus pruebas
de propiedad se mantienen fuera de la documentación pública.

## Flujo de alta

1. iOS encuentra HAVIT por BLE, se conecta con Espressif Security 1 y valida el
   código de posesión de seis dígitos que muestra la placa.
2. Antes de enviar Wi-Fi, iOS lee havit-info, valida protocolo, identidad
   completa e identity_provisioned.
3. iOS solicita prepare_claim en havit-claim, guarda la prueba temporal en
   Keychain con atributo ThisDeviceOnly y vuelve a leerla para verificarla.
4. iOS calcula el hash acordado y envía ack_claim. Wi-Fi solo se envía si la
   respuesta es acknowledged: true.
5. La placa se conecta a Wi-Fi y publica su sesión con su credencial individual
   instalada por USB en la partición havit_identity.
6. Después de iniciar sesión, iOS presenta la prueba guardada junto con el
   Firebase ID token. El backend verifica cuenta, credencial y prueba; al
   aceptar crea un bindingId.
7. App y placa muestran «Dispositivo vinculado» tras confirmar el vínculo con
   el servidor. Un nombre BLE o un ID por sí solos no prueban propiedad.

El PIN de seis dígitos verifica posesión una vez durante el aprovisionamiento.
No se necesita un segundo PIN para iniciar sesión, sincronizar o reiniciar la
placa. Si BLE se cierra o iOS se reinicia, la prueba pendiente permanece en
Keychain hasta que la reclamación se confirme o caduque.

## Almacenamiento en el ESP32

- havit_identity: identidad y credencial propia de la unidad, cargadas por una
  herramienta de operador. El backend conserva el hash de la credencial.
- havit_claim: sesión de prueba/ACK y estado de vinculación en el firmware.
- havit_tasks: snapshot y operaciones pendientes de actividades.
- NVS de Wi-Fi: SSID y configuración de red del aprovisionamiento.

El mapa real de particiones está en firmware/partitions.csv. La identidad,
la red Wi-Fi, el estado de propiedad y las tareas ocupan áreas distintas.
Actualizar firmware no debe borrar esas particiones. La preparación de
identidad es un procedimiento de operador: sigue la
[guía de backend](https://github.com/romdesigns0-dotcom/HAVIT-APP/blob/main/backend/README.md)
y guarda identity.csv/identity.bin fuera de ambos repositorios.

## Sincronización normal

- Con Wi-Fi, el ESP32 llama por HTTPS a GET /v1/device/sync y
  POST /v1/device/events. La app accede a la misma cuenta por HTTPS usando
  Firebase Authentication.
- Sin Wi-Fi en el ESP32, HAVIT puede transferir mensajes cifrados por BLE a la
  app. El iPhone los retransmite al endpoint HTTPS con Internet móvil o Wi-Fi.
  Este relay requiere la app en primer plano; iOS no garantiza que se ejecute
  continuamente suspendida.
- Sin Internet en ambos lados, las copias y colas quedan locales. Se envían
  cuando vuelve una conexión autorizada.
- Los mensajes BLE del relay usan AES-256-GCM y se validan en servidor contra
  el propietario, el bindingId, la credencial revocable y un nonce reciente.
  Security 1 del aprovisionamiento no sustituye esta autorización de tareas.

Contratos: [API y alta de dispositivo](https://github.com/romdesigns0-dotcom/HAVIT-APP/blob/main/backend/README.md),
[sincronización de actividades](https://github.com/romdesigns0-dotcom/HAVIT-APP/blob/main/docs/TASK-SYNC-IMPLEMENTATION.md)
y [relay BLE](https://github.com/romdesigns0-dotcom/HAVIT-APP/blob/main/docs/TASK-BLE-RELAY.md).

## Restablecimiento y cambio de cuenta

- **Restablecer Wi-Fi:** borra las credenciales de red y permite configurar
  otra red. No desvincula ni rota la identidad de fábrica.
- **Cerrar sesión:** termina la sesión de iOS; no cambia propietario del ESP32.
- **Desvincular/restablecer de fábrica:** la cuenta propietaria autoriza una
  solicitud con su Firebase ID token y bindingId. La placa confirma la
  limpieza por Wi-Fi o por relay BLE si está offline. El servidor finaliza la
  desvinculación después del acuse y elimina los datos de cuenta solicitados.
  La identidad de fábrica permanece para requerir una nueva prueba BLE.
- **Nueva cuenta:** necesita una prueba física BLE nueva y un nuevo bindingId.
  La cuenta anterior no debe conservar caché, colas ni permisos.

La recuperación de una cuenta original inaccesible requiere el procedimiento
de operador; un reset local no debe permitir apropiarse de un dispositivo
remotamente. La aceptación completa A → desvincular con placa offline →
reclamar desde B debe comprobarse con dos cuentas de prueba antes de considerar
cerrado el ciclo de propiedad.

## Desarrollo y diagnóstico

El firmware se implementa en C/C++ sobre ESP-IDF 5.4.2, FreeRTOS y LVGL 9.5.
La pantalla y los controles de la Waveshare ESP32-S3-Touch-LCD-1.85 se
inicializan en hardware/; servicios de red y estado viven en services/; las
pantallas LVGL están en ui/modules/.

Desde PowerShell con el entorno ESP-IDF activado:

    idf.py -C firmware build
    idf.py -C firmware -p COMx flash
    idf.py -C firmware -p COMx monitor

Usa el puerto que Windows asignó a la placa. No ejecutes erase-flash para una
actualización normal. Para el flujo iOS/WSL y la arquitectura conjunta, consulta
el README del dispositivo y
[ARCHITECTURE.md de HAVIT-APP](https://github.com/romdesigns0-dotcom/HAVIT-APP/blob/main/ARCHITECTURE.md).

Los scripts de diagnóstico BLE requieren seleccionar explícitamente nombre,
identidad y bindingId de la unidad de prueba. No escribas secretos en
argumentos visibles, consola, logs ni repositorio.
