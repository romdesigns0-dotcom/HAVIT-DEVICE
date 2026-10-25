# Vinculación de HAVIT

## Flujo acordado

1. iOS encuentra HAVIT por BLE y verifica el PoP de seis dígitos una vez.
2. Dentro de Security 1, iOS lee `havit-info`, obtiene `device_id` y solicita `prepare_claim`.
3. iOS conserva la prueba de 32 bytes en Keychain y la vuelve a leer. Envía `ack_claim`; HAVIT persiste sesión/hash/ACK en su partición dedicada y borra el secreto de RAM.
4. iOS envía Wi-Fi por BLE. HAVIT se conecta y publica la prueba confirmada por HTTPS.
5. La persona inicia sesión en HAVIT móvil. iOS obtiene Firebase ID token y llama a `POST /v1/devices/claim` con la prueba de Keychain.
6. HAVIT consulta `GET /v1/device/link` por Wi-Fi y espera respuesta `linked` antes de mostrar el acceso a Today.

No hay otro PIN, no se confirma propiedad con un toque y el teléfono no necesita seguir conectado por BLE. El protocolo de actividades todavía no está implementado; por eso la pantalla vinculada no dice que las tareas ya se sincronizaron.

## Implementación del firmware

- `services/connectivity.c`: configura BLE/Wi-Fi y publica los estados de red.
- `services/device_claim.c`: endpoint BLE cifrado, NVS de identidad/reclamación y cliente HTTPS de vinculación.
- `firmware/partitions.csv`: reserva `havit_identity` en `0xC10000` con tamaño `0x6000`; la NVS normal no se mueve.
- La identidad abre el namespace `havit_identity`, igual al CSV de `register-device.ts`, y valida que `device_id` coincida con el MAC completo anunciado por `havit-info`.
- Restablecer Wi-Fi no invalida el ACK ni borra el binding. Después de reiniciar, el binding guardado se trata como caché hasta que `GET /v1/device/link` lo confirme con `state: linked` y `bindingId`.
- El vencimiento ISO devuelto al publicar queda persistido. Después de un reinicio, el firmware sincroniza hora antes de decidir si la prueba venció. `Retry-After` y `retryAfterSeconds` se guardan para evitar reintentos antes del plazo.
- HTTPS exige hora válida por SNTP y valida certificados con el bundle completo de ESP-IDF.
- Sin credencial individual válida no se llama a la API y la UI indica alta USB pendiente.

## Ciclo de propiedad — cambio local pendiente de despliegue

- Restablecer Wi-Fi no toca el dueño, la identidad ni el ACK persistido. Cerrar sesión en iOS tampoco llama al endpoint de unlink.
- El backend devuelve `unclaimed` después de la desvinculación. Si la placa estuvo offline, rechaza por `ownershipEpoch` cualquier prueba de una generación anterior; el firmware elimina binding y sesión BLE guardados antes de pedir una ventana BLE nueva.
- Si el siguiente `bindingId` difiere del que quedó en NVS, el firmware purga la prueba anterior y conserva la identidad de la unidad y la configuración Wi-Fi.
- No hay caché/cola local de tareas en el firmware; se limpia solo el estado local de vinculación. La sincronización de tareas se implementará después con namespace por dueño y binding.
- Esta rama aún no se ha escrito en el ESP32. La prueba A → unlink con Wi-Fi desconectado → reconnect → B requiere que el backend actualizado esté desplegado y usar dos cuentas de prueba.
- Compilación incremental con ESP-IDF 5.4.2 tras este cambio: correcta. `build-ownership-cycle/havit_os_firmware.bin`, `0x40B950` (4,241,744 bytes); quedan 8,341,168 bytes (66 %) en la partición de aplicación de 12 MiB. No se flasheó la placa ni se cambió su NVS.

## Validación de firmware y placa — 2026-10-22

- Compilación limpia con ESP-IDF 5.4.2, ESP32-S3: correcta. `havit_os_firmware.bin` mide 4,241,024 bytes (`0x40B680`); quedan 8,341,888 bytes (66 %) en la partición de aplicación de 12 MiB.
- `device_id` correcto, confirmado en el arranque: `havit-2884859195aa` (MAC Bluetooth de la unidad). No usar la MAC base que devuelve `esptool read_mac` para este identificador.
- El registro correcto quedó activo en Firebase. Un registro preliminar con el ID derivado de la MAC base se revocó; no se usa en la placa. Sus archivos locales privados permanecen revocados y fuera del repositorio.
- La herramienta de operador verificó la tabla instalada y escribió solo `havit_identity` en `0xC10000` (`0x6000` bytes). La lectura de vuelta coincide con el binario privado; el namespace `havit_identity`, el ID y el formato del token se validaron sin imprimir la credencial.
- Se escribió únicamente la aplicación en `0x10000`; esptool verificó el hash. El rango escrito termina en `0x41BFFF`. No se reescribieron bootloader ni tabla y no se tocó la NVS estándar de `0x9000` ni los datos Wi-Fi.
- El monitor serie confirma arranque con ESP-IDF 5.4.2, tabla correcta, PSRAM de 8 MiB, panel ST77916, táctil CST816S (ID 181), identidad cargada para el ID correcto e inicio del anuncio BLE `HAVIT-95AA`.
- La prueba BLE desde Windows encontró `HAVIT-95AA`, el servicio de provisión, endpoints `havit-info` y `havit-claim`, y negociación Security 1 + PoP. No se enviaron credenciales Wi-Fi ni se hizo el ACK de una sesión real.
- La API respondió HTTP 200 `unclaimed` usando la credencial recién registrada; la credencial no se imprimió. El dispositivo sigue sin vincular a una cuenta y no hay sincronización de tareas.

La unidad queda encendida y anunciando BLE para que iOS la pruebe. Todavía falta completar desde iOS `havit-info` → `prepare_claim` → Keychain → `ack_claim` → Wi-Fi → reclamar después del login, y verificar el estado `linked` consultado por el firmware. No uses `nvs_flash_erase()`.
