# Servicios del firmware

- `connectivity.c`: inicia Wi-Fi, el gestor oficial de provisión BLE de ESP-IDF y publica un snapshot de estado para LVGL.
- `device_claim.c`: carga la identidad individual desde el namespace NVS `havit_identity`, sirve `havit-claim`, persiste el ACK y publica/consulta la vinculación con HTTPS.

La partición normal `nvs` no se borra. La identidad y el ACK usan la partición `havit_identity` (offset `0xC10000`, tamaño `0x6000`) y el namespace de identidad `havit_identity`, igual al CSV creado por `register-device.ts`. El namespace `havit_claim` contiene la prueba temporal, su vencimiento, el binding confirmado y el próximo reintento. Restablecer Wi-Fi solo reemplaza SSID/contraseña; conserva identidad, ACK y binding. La tabla debe estar instalada antes del alta USB; una carga solo de aplicación no instala la partición nueva.

La identidad se obtiene únicamente del procedimiento de operador descrito en `XTOOL/Havit/backend/README.md`: `device_id`, `device_token` y `api_base_url`. El token nunca se incluye en respuestas BLE, logs o el repositorio. Si no existe una identidad válida para esta unidad, el servicio BLE local sigue funcionando, pero el firmware no hace peticiones autenticadas al backend. El flujo USB actual es un prototipo y guarda la partición sin cifrar; no debe usarse para fabricación hasta habilitar cifrado de flash/NVS y acordar recuperación segura.

Endpoint `havit-claim`, dentro de Security 1:

- `prepare_claim`: devuelve la misma sesión y secreto durante la ventana actual; el secreto es aleatorio de 32 bytes y no se persiste antes del ACK.
- `ack_claim`: comprueba el hash acordado, guarda ID/hash/ACK en NVS dedicada y después limpia el secreto de RAM.
- Un reinicio conserva el ACK y la expiración recibida del servidor. Tras recuperar Wi-Fi, no se vuelve a publicar una sesión vencida; el firmware consulta primero el estado del binding. Una nueva solicitud BLE `prepare_claim` puede crear otra prueba después de la expiración.
- Con Wi-Fi, publica una sesión confirmada mediante `POST /v1/device/claim-sessions`, y consulta `GET /v1/device/link` cada 15 segundos. Solo una respuesta HTTPS válida con el `deviceId` correcto, `state: linked` y un `bindingId` no vacío marca `account_linked`; una copia local de NVS no basta después de reiniciar.
- Los errores distinguen sesión pendiente, prueba caducada, credencial rechazada y límite de solicitudes. El firmware procesa `Retry-After`/`retryAfterSeconds` y conserva el plazo de espera entre reinicios. Las credenciales rechazadas no se borran ni regeneran automáticamente.

El firmware usa el almacén de certificados de ESP-IDF y sincroniza hora por SNTP antes de abrir HTTPS. No se omite la validación TLS. Las rutas de actividades no están implementadas; la pantalla permite entrar al flujo existente después de una vinculación real, pero no afirma que las tareas ya se hayan sincronizado.

Contrato iOS/API y pruebas físicas pendientes: [BLE_WIFI_IOS.md](../docs/BLE_WIFI_IOS.md).
