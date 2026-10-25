# HAVIT: aprovisionamiento BLE y Wi-Fi real

## Alcance

El firmware usa ESP-IDF 5.4.2, `wifi_provisioning`, NimBLE y protocomm Security 1 (X25519 + AES-CTR + prueba de posesión). BLE transporta las credenciales; Wi-Fi obtiene una dirección IP del router. Obtener una IP no demuestra que Internet o el backend estén disponibles.

El firmware implementa el endpoint cifrado `havit-claim`, la confirmación `ack_claim` con persistencia en NVS dedicada, la publicación autenticada de la sesión y la consulta del estado real de vinculación por HTTPS. La app iOS todavía debe leer `havit-info`, guardar la prueba en Keychain, calcular su hash, enviar `ack_claim` y llamar a la API después de iniciar sesión. Las rutas de tareas no están implementadas.

## Cómo probar en HAVIT

1. Arranca la placa. Sin red guardada, anuncia BLE automáticamente durante una ventana de 10 minutos.
2. Toca los ojos y la flecha. «Conecta tu HAVIT» informa si hay una red guardada y ofrece «Ver / iniciar conexión» o «Restablecer Wi-Fi». Para empezar desde cero, selecciona restablecer y confirma «Borrar y configurar». Solo entonces se borran SSID/contraseña y se abre BLE con el nombre `HAVIT-XXXX` y un código de seis dígitos. Puedes cancelar antes de borrar.
3. Desde la app, busca ese nombre y solicita la conexión segura. Introduce exactamente los seis dígitos, conservando ceros iniciales. El código cambia al reiniciar o abrir una sesión de configuración nueva; no se guarda en Git ni se imprime por USB.
4. Pide el escaneo desde la app. El ESP32-S3 devuelve las redes que él alcanza. Elige una red de 2.4 GHz e introduce su contraseña en el teléfono.
5. La pantalla cambia a «Conectando a Wi-Fi» solo cuando recibe las credenciales. «Wi-Fi conectado» aparece al obtener una IP real por DHCP.
6. Tras obtener IP, HAVIT publica por HTTPS una prueba que iOS confirmó que guardó en Keychain. El estado pasa a «Esperando a HAVIT móvil» hasta que la app inicie sesión y el servidor vincule el dispositivo. Un toque no confirma propiedad.
7. Cuando `GET /v1/device/link` informa `linked`, HAVIT muestra «Dispositivo vinculado». Continuar abre las vistas locales actuales; la API de tareas sigue pendiente y no se afirma que se hayan sincronizado actividades.
8. Reinicia: la placa usa la red de NVS y se reconecta. Con red guardada no vuelve a anunciar BLE automáticamente. «Ver / iniciar conexión» muestra el estado real de esa red sin simular los pasos Bluetooth. Para cambiarla, usa «Volver» → «Restablecer Wi-Fi» → «Borrar y configurar». Se eliminó el gesto oculto de pulsación larga.

El gestor reemplaza la configuración Wi-Fi al recibir otra red. No se borra la partición NVS ni el resto de datos. Si NVS requiere recuperación, se muestra un error y se conserva en lugar de ejecutar `nvs_flash_erase()`.

## Contrato para la app iOS

| Campo | Valor |
|---|---|
| Biblioteca | ESPProvision de Espressif |
| Transporte | `.ble` |
| Seguridad | `.secure` (Security 1), nunca `.unsecure` |
| Prefijo de búsqueda | `HAVIT-` |
| Nombre mostrado | `HAVIT-` + últimos dos bytes de MAC Bluetooth en hexadecimal mayúscula |
| Identidad completa | `havit-` + los seis bytes de MAC Bluetooth, hexadecimal minúscula sin separadores |
| Código / PoP | seis dígitos mostrados por la placa, cadena de texto |
| UUID de servicio | `1775244d-6b43-439b-877c-060f2d9bed07` (gestor de IDF 5.4.2) |
| Endpoints estándar | `proto-ver`, `prov-session`, `prov-scan`, `prov-config`, `prov-ctrl` |
| Endpoint adicional cifrado | `havit-info` |
| Endpoint cifrado de reclamación | `havit-claim` |
| Ventana BLE | 10 minutos sin completar; al conectar Wi-Fi el gestor cierra BLE tras consultar éxito / timeout de 30 s |

El nombre corto ayuda a elegir una placa cercana, pero puede repetirse. Usa `device_id` completo como identificador de integración; la MAC no es una credencial ni prueba de propiedad.

El código Bluetooth protege el aprovisionamiento y se pide **antes de seleccionar/enviar la red**. Es distinto del PIN opcional de reclamación de cuenta propuesto en el diseño: ese segundo mecanismo todavía no está implementado. En HAVIT no hay teclado; la app usa su teclado numérico.

## Llamadas de ESPProvision

Añade el paquete oficial en Xcode: <https://github.com/espressif/esp-idf-provisioning-ios>. Añade `NSBluetoothAlwaysUsageDescription` con una explicación del uso de Bluetooth. La app debe manejar permiso denegado y Bluetooth apagado. Para este flujo BLE no necesitas leer el SSID actual del iPhone: muestra la lista escaneada por HAVIT. No implementes el cifrado ni el protocolo GATT a mano.

```swift
import ESPProvision

// 1. Buscar. Conserva una referencia fuerte al ESPDevice seleccionado.
ESPProvisionManager.shared.searchESPDevices(
    devicePrefix: "HAVIT-", transport: .ble, security: .secure
) { devices, error in
    // Mostrar devices; al seleccionar uno, conservarlo como `device`.
}

// 2. Delegado retenido por tu coordinador durante toda la operación.
final class HavitSessionDelegate: ESPDeviceConnectionDelegate {
    let code: String
    init(code: String) { self.code = code }

    func getProofOfPossesion(forDevice: ESPDevice,
        completionHandler: @escaping (String) -> Void) {
        completionHandler(code) // seis dígitos capturados con teclado numérico
    }

    func getUsername(forDevice: ESPDevice,
        completionHandler: @escaping (String?) -> Void) {
        completionHandler(nil) // Security 1 no utiliza username
    }
}

// 3. `device` es el ESPDevice elegido; retén también `sessionDelegate`.
// device.connect(delegate: sessionDelegate) { status in
//     switch status {
//     case .connected: ... // ahora permite buscar redes
//     case .failedToConnect(let error): ... // revisar código / reconectar
//     case .disconnected: ...
//     }
// }

// 4. Tras .connected:
// device.scanWifiList { networks, error in ... }

// 5. Tras elegir SSID y escribir la contraseña:
// device.provision(ssid: selectedSSID, passPhrase: password) { status in
//     switch status {
//     case .configApplied: ... // aún no es éxito; esperar DHCP
//     case .success: ...       // Wi-Fi configurado; no implica cuenta vinculada
//     case .failure(let error): ...
//     }
// }
```

Los fragmentos son una guía de integración basada en las firmas oficiales; no se han compilado dentro de una app iOS en esta ejecución. Actualiza la interfaz de la app en el hilo principal. Evita operaciones BLE simultáneas: espera la respuesta de cada comando antes de enviar el siguiente. No registres el código ni la contraseña; revisa/desactiva los logs de ESPProvision al integrarlo, pues su código contiene mensajes de depuración con esos valores.

## Identidad antes de aprovisionar

Después de establecer la sesión segura y antes de `provision`, puedes obtener la identidad:

```swift
// Ejecutar con la sesión .secure conectada y antes de `provision`.
device.sendData(path: "havit-info", data: Data("{}".utf8)) { data, error in
    // Validar protocol, device_id, name e identity_provisioned.
    // Si identity_provisioned es false, pedir alta de operador; no enviar Wi-Fi
    // ofreciendo una vinculación que el backend no puede autorizar.
}
```

Respuesta de ejemplo, sin identidad ni credenciales reales:

```json
{"protocol":1,"device_id":"havit-001122334455","name":"HAVIT-4455","wifi_connected":false,"ip":"","presence_confirmed":false,"account_linked":false,"identity_provisioned":true}
```

El endpoint solo funciona durante la sesión BLE cifrada. No dependas de que siga disponible después de `.success`: el gestor puede cerrar BLE al responder el estado de Wi-Fi. La app conoce el éxito de Wi-Fi por `provision` y el estado de la cuenta por la API.

## Reclamación BLE que iOS debe completar

Debe ocurrir después de Security 1 y antes de mandar SSID/contraseña. Conserva el mismo `ESPDevice` y espera cada respuesta antes de la siguiente operación:

1. Lee `havit-info`; compara `device_id` con la identidad que el usuario seleccionó. Si `identity_provisioned` es `false`, detén el flujo: la unidad necesita alta por USB.
2. Envía `{"protocol":1,"op":"prepare_claim"}` a `havit-claim`. Guarda `device_id`, `claim_session_id`, `claim_secret`, `expires_in` y la versión de protocolo en Keychain con `kSecAttrAccessibleWhenUnlockedThisDeviceOnly`. Vuelve a leer y verifica la misma entrada.
3. Decodifica `claim_secret` como base64url sin relleno. Calcula SHA-256 sobre los bytes UTF-8 de `havit-claim-v1:` + `device_id` + byte NUL + `claim_session_id` + byte NUL + los 32 bytes secretos. Envía `{"protocol":1,"op":"ack_claim","claim_session_id":"…","claim_secret_hash":"<64 hex minúscula>"}`. Continúa solo si devuelve `acknowledged: true`.
4. Solo entonces llama a `device.provision(ssid:passPhrase:)`. El secreto original permanece en Keychain para presentarlo a la API después de iniciar sesión; el dispositivo conserva únicamente ID/hash/ACK y publica el hash con su credencial propia.
5. Tras login/registro, iOS obtiene el Firebase ID token y llama a `POST /v1/devices/claim` con `deviceId`, `claimSessionId` y `claimSecret`. Reintenta `409 CLAIM_NOT_READY` con el `retryAfterSeconds` indicado mientras la prueba siga vigente. Al éxito, borra la prueba de Keychain.

La petición y respuesta BLE completas, errores, expiración y esquema HTTP están en `XTOOL/DEVICE-API-CONTRACT.md`, sección 4. El firmware no imprime PoP, secreto, hash ni token de unidad. Una repetición idéntica de `prepare_claim` dentro de la ventana devuelve la misma prueba; después del ACK devuelve solo metadatos. Una sesión BLE nueva tras reinicio no recupera un secreto que nunca recibió el ACK. Al abrir deliberadamente otra ventana BLE desde Wi-Fi restablecido, HAVIT descarta una prueba anterior solo si no está vinculada, para que iOS pueda guardar y confirmar una nueva. Una unidad vinculada no abre otra reclamación.

### Recuperar un ACK anterior que no está en Keychain

Si `prepare_claim` devuelve `state: acknowledged` y el iPhone no conserva el secreto de ese `claim_session_id`, iOS debe llamar por el mismo endpoint BLE cifrado a `{"protocol":1,"op":"restart_claim"}`. El firmware comprueba que la identidad exista y que no tenga un vínculo confirmado, elimina solo la prueba pendiente y responde con una prueba nueva que iOS guarda y confirma antes de Wi-Fi. La autenticación de cuenta y la propiedad única siguen validándose en el backend. Esta recuperación evita pedir que se borre Wi-Fi o que se use USB.

## Estado del alta de unidad y comprobaciones físicas

La placa aún no tiene la credencial propia del backend. Antes del recorrido real hay que:

1. Identificar el `device_id` completo mostrado por `havit-info`.
2. Dar de alta exactamente esa unidad con la herramienta de operador y generar el archivo local `identity.csv` fuera del repositorio.
3. Programar la tabla de particiones que incorpora `havit_identity` en `0xC10000` y escribir la imagen de identidad únicamente en esa partición, según `Havit/backend/README.md`.

La tabla reserva 24 KiB al final de la app de 12 MiB; la NVS normal conserva su dirección y contenido. Un `app-flash` no instala la tabla modificada. La compilación se completó; todavía no se han flasheado la tabla nueva ni el firmware, y no se ha escrito una credencial en el dispositivo.

- Código incorrecto: desconecta y vuelve a conectar con el código visible. Si caducó la ventana, pulsa «Reintentar» o usa «Volver» → «Restablecer Wi-Fi» para abrir otra sesión tras confirmar.
- Contraseña incorrecta / red no encontrada: HAVIT muestra el error. Pulsa «Reintentar» en HAVIT para liberar el estado de fallo; después vuelve a enviar la configuración desde la app. En versiones de ESPProvision que lo expongan, `resetWifiStatus` realiza el reinicio del estado de provisión desde el móvil.
- Pérdida de red tras configurar: se reconecta periódicamente; las pantallas no afirman que haya sincronización con el backend.
- Token de unidad ausente o revocado: no se envían peticiones autenticadas; el estado muestra que el alta USB está pendiente o que el backend no pudo confirmar.
- Probar: búsqueda con dos HAVIT, código incorrecto, contraseña incorrecta, conexión correcta, reinicio y reconexión, router apagado y recuperado, y cambio explícito de red.

## Fuentes y organización

- Gestor oficial de IDF: <https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/api-reference/provisioning/wifi_provisioning.html>.
- API iOS: <https://github.com/espressif/esp-idf-provisioning-ios> y fuentes `ESPDevice.swift` / `ESPProvisionManager.swift` del mismo repositorio.
- `services/connectivity.c`: radios, NVS, estado y eventos. La tarea de red nunca llama LVGL.
- `ui/modules/login_flow.c`: consulta una copia del estado cada 100 ms desde la tarea LVGL.
- `ui/modules/device_setup.c`: objetos gráficos reutilizados; sin fondo ni aro propios.
- `tools/probe_ble_provisioning.py`: verificación de anuncio, GATT y capacidades sin enviar contraseñas.

## Corrección del flujo

Las vistas de conexión no se recorren mediante toques de demostración. Si se usa la red guardada, se muestra su conexión directamente y se explica su origen; si se restablece, el recorrido comienza en Bluetooth y espera los eventos reales de la app. Hay un botón «Volver» en las pantallas de estado. No se muestra una cuenta autenticada al terminar Wi-Fi. Los textos tienen bandas separadas y límites de altura; el cuerpo se ajusta con las métricas de Instrument Sans para no invadir botones.

## Estado de implementación (2026-10-21)

Compilación final correcta: 4,022,816 bytes (0x3d6220), 68 % libre en la partición de aplicación. Carga por COM5 con verificación de hash, solo aplicación desde 0x10000. Se comprobó reconexión real de la red previamente guardada y obtención de IP durante la integración. En la revisión final LVGL siguió activo a los 31 y 41 segundos, con 93,415 bytes de heap interno libre y 6,812,344 bytes de PSRAM libre en esas muestras, sin pánico en el intervalo observado. No son métricas de pico ni una prueba prolongada.

Pendiente de prueba física: confirmar el restablecimiento desde la nueva interfaz, descubrimiento BLE desde el teléfono, handshake con el código, contraseña incorrecta y aprovisionamiento de una red nueva. El primer escaneo desde Windows no encontró anuncios porque la placa estaba usando su red guardada; no se considera una validación de BLE. El script de comprobación está listo para ejecutarse cuando el usuario abra la nueva sesión BLE.


El firmware recompiló con `havit-claim`, ACK, persistencia dedicada, cliente HTTPS, SNTP y consulta de vinculación. `havit_os_firmware.bin` mide 4,236,000 bytes (`0x40A2E0`); quedan 8,346,912 bytes libres (66 %) en la partición de aplicación de 12 MiB. La tabla generada muestra `havit_identity` en `0xC10000`, tamaño 24 KiB, sin mover la NVS normal. La placa aún no se ha flasheado con la tabla nueva y no tiene su identidad USB. Por eso no se ha validado el recorrido BLE → Keychain → ACK → Wi-Fi → API.

La siguiente prueba requiere, en orden: registrar el `device_id` exacto de la placa con el operador; escribir solo `havit_identity`; flashear el firmware de integración sin borrar la NVS normal; instalar la app iOS actual; completar BLE → Keychain → ACK → Wi-Fi → login → reclamación y repetir tras reiniciar ESP32/cerrar la app. La integración iOS está implementada e instalada. No usar un binario o archivo de identidad de otra unidad. Las rutas de tareas siguen pendientes.
