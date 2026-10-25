# Contexto compartido HAVIT: iOS, ESP32 y Firebase

Estado vigente: consultar `../CONTEXTO_INTEGRACION_HAVIT.md` y `../XTOOL/TASK-BLE-RELAY.md`. Este archivo conserva notas históricas anteriores a la sincronización de actividades y al puente BLE.

Actualización de compatibilidad 2026-10-22: se comprobó en COM5 que la imagen instalada era `build-ownership-cycle` (04:57:35, sin `restart_claim`). Se cargó únicamente la aplicación de `build-claim-recovery` en `0x10000`, hash verificado; NVS, identidad, tabla y bootloader conservados. iOS con recuperación y mensajes de error específicos compiló y se instaló/verificó en el iPhone. La placa arrancó con identidad y quedó preparada para BLE (`screen=4 network=1 identity=1 wifi=0 linked=0`). Sigue pendiente repetir el intercambio desde iPhone y comprobar ACK, Wi-Fi y reclamación. Ver la sección de compatibilidad al inicio de `../CONTEXTO_INTEGRACION_HAVIT.md` para la evidencia actual; los estados anteriores de «no flasheado/no instalado» son históricos.

Revisión del código local: 22 de octubre de 2026. Este documento es el punto de entrada para continuar el desarrollo de ambos proyectos. Distingue implementación, evidencia histórica y propuestas. No se consultó producción ni se probó hardware durante este análisis.

## 1. Producto y responsabilidades

HAVIT combina una app de iPhone para cuenta, configuración y organización con un dispositivo circular ESP32 que muestra actividades y permite interacciones rápidas. Ambos deben compartir los datos de la misma cuenta mediante una API respaldada por Firestore.

```text
iPhone / SwiftUI ── HTTPS + Firebase ID token ──┐
                                             ├─ Cloud Functions ─ Firestore
ESP32 / LVGL ────── HTTPS + token de unidad ───┘
       ↕ BLE Security 1 durante configuración
      iPhone
```

La conexión permanente entre los productos es la API. BLE configura Wi-Fi y entrega una prueba temporal de posesión; el teléfono no necesita permanecer cerca después. Firebase Authentication identifica a la persona; Firestore conserva la vinculación y, cuando se implemente, los datos compartidos. El asistente coordina contratos y cambios en los tres componentes, pero no es un servicio que ejecute solicitudes en producción.

## 2. Mapa del código y estado real

| Componente | Ubicación | Estado observado |
| --- | --- | --- |
| Firmware | `lvgl_vscode_project/` | ESP-IDF, C/C++, LVGL; configuración BLE/Wi-Fi, identidad y cliente HTTPS de vinculación implementados |
| App | `XTOOL/Havit/` | SwiftUI, iOS 17+, Firebase Auth, ESPProvision 3.2.0; tareas locales |
| Backend | `XTOOL/Havit/backend/` | TypeScript, Express, Cloud Functions 2, Node 22, Firebase Admin/Firestore; vinculación implementada |
| Contrato API | `XTOOL/DEVICE-API-CONTRACT.md` | Detalla vinculación y propuesta de tareas; algunos estados de firmware quedaron desactualizados |
| Contrato sync | `XTOOL/DEVICE-SYNC-CONTRACT.md` | Propuesta, no implementación de sincronización |

No confundir `XTOOL/tmp/`, referencias, cachés de compilación o dependencias con el código principal. Ambos repositorios tienen cambios locales previos a este análisis: conservarlos.

El README del firmware todavía describe una pantalla negra, pero `core/app.cpp` inicia los ojos y `firmware/components/havit_ui/CMakeLists.txt` compila las vistas de configuración, Today, calendario, tarea rápida y equipos. Para saber qué existe, priorizar las fuentes y el manifiesto de compilación sobre ese README.

## 3. App iOS: pantallas y navegación existentes

Entrada y navegación: `Sources/Havit/ContentView.swift`.

```text
WelcomeView
 ├─ Empezar → PairDeviceView → FoundDeviceView (PoP de 6 dígitos)
 │                          → WiFiSetupView → WiFiConnectingView → LoginView
 └─ Iniciar sesión → LoginView
Login / continuar sin cuenta → HavitHomeView
Home → Crear actividad (sheet) / Cuenta (sheet) / Reiniciar recorrido
```

- `AuthSession.swift`, `LoginView.swift`, `HavitCredentialView.swift`: correo/contraseña, registro, recuperación, enlace de correo, Google y teléfono. La disponibilidad efectiva depende de los proveedores configurados; no se auditó la consola.
- `BluetoothController.swift`: descubrimiento BLE. `WiFiProvisioningController.swift`: sesión BLE Security 1, lectura y validación de `havit-info`, prueba/Keychain/ACK y luego envío de credenciales Wi-Fi. Logs detallados de ESPProvision están desactivados. Si el ESP conserva un ACK cuya prueba falta en Keychain, el código local ahora solicita `restart_claim` para crear una prueba nueva.
- `HavitDeviceLinkCoordinator` reclama por HTTP después del login con Firebase. `sessionReady` confirma validación y ACK BLE; por sí solo no demuestra que el servidor haya vinculado la unidad.
- `HomeView.swift`: actividades con `id`, `time` textual, `title`, `detail`, `done`, codificadas en la clave global `havit.home.tasks` de UserDefaults. No están separadas por usuario. Una bandera de limpieza inicial puede eliminar el contenido previo al primer acceso de esta versión.
- Inicio/Tareas/Calendario/Perfil son controles visibles; cambiar `selectedTab` no constituye una implementación de cuatro módulos independientes. Perfil abre la cuenta. El saludo de Home contiene un nombre fijo.
- La creación usa la hora actual formateada: no representa una fecha de vencimiento. No convertirla automáticamente en `dueAt` durante la migración.
- `LiveActivityDemo.swift`, `NotificationDemo.swift`, `AudioFocusController.swift`, `FocusSetupView.swift`, widget y modelo `Sources/Shared/FocusActivityAttributes.swift`: herramientas de estudio y presentación local. Hay vistas auxiliares en ContentView que no forman parte del dashboard activo; verificar alcanzabilidad antes de asumir una pantalla disponible.
- `Package.swift` enlaza FirebaseCore/Auth, no FirebaseFirestore. La app no tiene repositorio remoto de tareas ni cliente para Device API v1.

El flujo actualizado pide autenticación antes de emparejar y consulta `GET /v1/user/devices`: entra al inicio si la cuenta ya tiene un HAVIT vinculado y abre vinculación si todavía no tiene. Continuar sin cuenta ofrece el modo local. Una cuenta puede tener varios HAVIT; su lista se obtiene del servidor.

## 4. Firmware: pantallas, servicios y límites

Placa documentada: Waveshare ESP32-S3-Touch-LCD-1.85, ST77916, CST816S, pantalla circular 360 × 360. ESP-IDF 5.4.2, LVGL 9.5, PSRAM. Entrada: `firmware/main/main.cpp` y `core/app.cpp`.

| Flujo | Fuentes | Alcance actual |
| --- | --- | --- |
| Ojos, entrada, configuración y bienvenida | `ui/modules/eyes_intro.c`, `login1.c`, `login_flow.c`, `device_setup.c` | Vistas y estados reales de conectividad/vinculación; nombres internos antiguos no prueban que se use un segundo PIN |
| Today → Agregar / calendario → día | `ui/modules/today_screens.c` | UI y navegación; contenido de demostración, sin repositorio compartido |
| Agregar → Tarea rápida → temporizador → completado → Today | `ui/modules/quick_task.c` | Nombre editable y temporizador local; micrófono visual, sin reconocimiento de voz |
| Agregar → Equipo → Generando → Timeline → Desglose → Enviar → Today | `ui/modules/team_flow.c` | Prototipo visual con datos de muestra; no genera fechas ni envía a personas |
| BLE y Wi-Fi | `services/connectivity.c` | Aprovisionamiento, NVS de red, reconexión y snapshots de estado |
| Identidad y reclamación | `services/device_claim.c` | `havit-claim`, SHA-256, ACK persistente, publicación y consulta HTTPS |

La tarea de red publica snapshots; solo la tarea LVGL modifica objetos gráficos. Mantener esa separación al conectar tareas remotas.

Identidad en partición NVS `havit_identity`, offset `0xC10000`, tamaño `0x6000`; NVS normal sigue en `0x9000`. Namespace de identidad con `device_id`, `device_token`, `api_base_url`; namespace `havit_claim` para reclamación/vinculación. Restablecer Wi-Fi no debe borrar identidad ni dueño. No usar borrado global de NVS como recuperación automática.

La documentación reciente del firmware informa compilación con reclamación, pero deja pendientes nueva tabla/identidad de la placa y prueba completa. No se verificó el estado físico actual. Un binario compilado y una tabla CSV no demuestran que la placa los tenga instalados.

## 5. Flujo de vinculación implementado

1. La unidad obtiene identidad y credencial propia mediante alta de operador por USB, previa al uso final. No deducir la identidad completa a partir del nombre corto.
2. La persona abre configuración en la placa. iOS descubre `HAVIT-XXXX` y verifica el PoP de seis dígitos con BLE Security 1. Conservar ceros iniciales.
3. Dentro de la misma sesión cifrada, iOS lee `havit-info`: validar `protocol`, `device_id`, `name`, `identity_provisioned`. Si falta identidad, mostrar que la unidad requiere preparación de operador.
4. iOS envía `{"protocol":1,"op":"prepare_claim"}` a `havit-claim`.
5. Guarda identidad, sesión, secreto y caducidad en Keychain con `kSecAttrAccessibleWhenUnlockedThisDeviceOnly`, sin iCloud; vuelve a leer y verificar.
6. Calcula el hash y envía `ack_claim`; continuar solo con `acknowledged: true`. La placa persiste sesión/hash/ACK antes de borrar el secreto original de RAM.
7. iOS entrega Wi-Fi; la placa obtiene IP, hora SNTP y publica el hash por HTTPS con su token propio. BLE puede cerrarse.
8. Después de login, iOS obtiene Firebase ID token y reclama con la prueba de Keychain. Reintenta `CLAIM_NOT_READY` respetando el plazo del servidor. Al éxito elimina la prueba local. Cada cuenta puede vincular varios HAVIT y utilizarla desde varios iPhone; una unidad corresponde a una cuenta. Compartirla con otras cuentas necesita membresías y permisos, que siguen pendientes.
9. La placa consulta su estado y muestra vinculado solo cuando la API responde `linked`. Después podrá iniciar sincronización, cuando exista su API.

No hay segundo PIN en el recorrido normal. Si `prepare_claim` devuelve `acknowledged` pero el iPhone no tiene la prueba exacta, `restart_claim` reinicia solo esa prueba temporal antes de generar otra. PoP, prueba temporal, token de unidad y Firebase ID token tienen funciones distintas y no se intercambian.

Hash común:

```text
SHA256(UTF8("havit-claim-v1:") || UTF8(device_id) || 0x00 ||
       UTF8(claim_session_id) || 0x00 || secret_bytes)
```

Secreto: 32 bytes aleatorios, base64url canónico sin relleno. Hash: 64 caracteres hexadecimales minúsculos. BLE usa `device_id`/`claim_session_id`; HTTP usa `deviceId`/`claimSessionId`: mapear explícitamente.

## 6. API existente y Firestore

Proyecto indicado por configuración/documentación: `havit-c8c1d`; región `us-central1`.

Base documentada: `https://us-central1-havit-c8c1d.cloudfunctions.net/havitDeviceApiV1`.

| Método y ruta | Credencial | Entrada / resultado |
| --- | --- | --- |
| `GET /v1/health` | Pública | Servicio y protocolo |
| `POST /v1/device/claim-sessions` | Bearer token propio | `protocol`, `claimSessionId`, `claimSecretHash`; 201 nueva, 200 repetición |
| `POST /v1/devices/claim` | Bearer Firebase ID token | `deviceId`, `claimSessionId`, `claimSecret`; devuelve `linked`, `bindingId`, `linkedAt` |
| `GET /v1/device/link` | Bearer token propio | `deviceId`, `state: unclaimed/linked`, `bindingId` |

Fuentes autoritativas: `backend/functions/src/app.ts`, `service.ts`, `index.ts`. El verificador usa `verifyIdToken(token, true)`. El servidor deriva el usuario del token y el dispositivo de su credencial; no acepta `uid` del cliente como autorización.

| Colección real | ID de documento | Datos principales |
| --- | --- | --- |
| `havitV1Devices` | deviceId | `credentialHash`, `active`, `ownerUid`, `bindingId`, `pendingSessionId`, tiempos en ms |
| `havitV1DeviceCredentials` | SHA-256 del token | `deviceId`, `revokedAtMs`, `createdAtMs` |
| `havitV1ClaimSessions` | SHA-256 de `deviceId + ":" + sessionId` | `deviceId`, `sessionId`, hash de prueba, caducidad, consumo y binding |
| `havitV1RateLimits` | SHA-256 de clave de límite | `minute`, `count` |

La transacción asigna un único dueño y permite repetición de la misma reclamación por ese dueño. Caducidad de servidor: 30 minutos desde la primera publicación; repetir no la amplía. Límite compartido de publicación/consulta: 60/min por dispositivo; reclamación: 10/min por usuario.

Errores con forma `{"error":{"code":"..."}}`: 401 credencial inválida/revocada, 403 prueba incorrecta o acceso prohibido, 409 no listo/ya vinculado/sesión sustituida, 410 caducado, 429 límite y 503 indisponibilidad. `CLAIM_NOT_READY` indica reintento de 5 s. Manejar códigos exactos por ruta; no asumir que todos los 401 significan sesión de usuario vencida.

Firestore queda detrás de Firebase Admin en las funciones. El archivo `firestore.test.rules` deniega todo acceso y es de pruebas; `firebase.json` configura despliegue de funciones, no reglas productivas. El README informa una denegación directa comprobada históricamente, pero esta revisión no valida las reglas desplegadas actuales.

Alta y revocación son herramientas de operador (`register-device.ts`, `revoke-device.ts`, `backend/scripts/flash_identity.py`), no endpoints de autoalta. No exponer tokens, secretos BLE, PoP, contraseñas Wi-Fi ni archivos privados de identidad en Git o logs.

El README documenta despliegue y pruebas HTTPS anteriores. Eso es evidencia histórica, no una comprobación de disponibilidad de hoy. También menciona funciones antiguas de watches; no migrar ni reutilizar sus colecciones sin revisar su contrato.

## 7. Actividades: contrato previsto, todavía no implementado

Modelo mínimo propuesto: `id` UUID estable, `title`, `detail`, `dueAt` UTC o null, `status` pending/completed, `taskVersion`, `updatedAt`. El servidor asigna versiones y revisión. El contador de Today se deriva de las tareas, no de un contador independiente enviado por cada cliente.

Rutas propuestas en `DEVICE-API-CONTRACT.md`:

- iOS: `GET /v1/tasks?sinceRevision=...`, `POST /v1/tasks`, `PATCH /v1/tasks/{id}`, `DELETE /v1/tasks/{id}`.
- ESP32: `GET /v1/device/sync?sinceRevision=...`, `POST /v1/device/events`.
- Escrituras con idempotencia; eventos con `eventId`, `bindingId`, `baseTaskVersion`; borrados con tombstones; snapshot paginado y cursor confirmado al terminar todas sus páginas.
- Si cambia `uid` o `bindingId`, invalidar caché, cursores y operaciones del dueño anterior. Una cuenta nunca hereda tareas locales de otra automáticamente.

No existen esas rutas en `app.ts`, ni colecciones de actividades v1 definidas en `service.ts`. Antes de implementarlas falta cerrar ubicación de documentos por usuario, índices, revisión transaccional, tamaño máximo de respuesta, paginación, retención de borrados/eventos y política de conflictos. No inventar un esquema de Firestore y presentarlo como desplegado.

Las tareas rápidas requieren decidir si son sesiones locales de concentración, actividades sincronizadas o ambas. Equipos necesita definir miembros, permisos, invitaciones, entregas y notificaciones. El modelo mínimo de tarea individual no cubre esos conceptos ni un motor de generación de planes.

## 8. Pantallas y estados por completar

| Prioridad | iOS | ESP32 / backend relacionado |
| --- | --- | --- |
| P0 | Preparando unidad, error de identidad, guardado seguro/ACK | Verificar alta USB y tabla instalada |
| P0 | Vinculando cuenta, esperando publicación, éxito real, prueba vencida, otra cuenta y reintento | Publicación y consulta reales; probar reinicios y caducidad |
| P1 | Estado de dispositivo dentro de Perfil: vinculado, sin conexión, pendiente | Hace falta definir consulta autorizada para usuario; no existe ruta de listado/estado del dispositivo para iOS |
| P1 | Lista/alta/edición/detalle de tareas con fecha y estado remoto | Repositorio Firestore y rutas de tareas; reemplazar datos demo de Today/calendario |
| P1 | Cargando, vacío, offline, cambios pendientes, error y conflicto | Caché y cola persistentes, deduplicación y borrados |
| P2 | Calendario funcional y detalle diario | Navegación real por mes y fechas reales en firmware |
| P2 | Sesión rápida/estudio conectada al modelo acordado | Persistencia y semántica del temporizador |
| P3 | Equipos, participantes, plan, confirmación/envío | Servicios y permisos de colaboración; actualmente demostración |
| Por definir | Desvincular, transferir o recuperar unidad | No existe endpoint de transferencia ni revocación por usuario |

Estas son necesidades funcionales propuestas, no aprobación de un diseño visual nuevo. Reutilizar las vistas existentes cuando permitan expresar el estado real.

## 9. Orden recomendado de implementación y aceptación

1. Completar cliente de reclamación iOS y estado de navegación; conservar la prueba al cerrar BLE y al reiniciar la app. Confirmar entrada sin cuenta como modo local.
2. Verificar identidad física y hacer la prueba BLE → Keychain → ACK → Wi-Fi → Auth → API → vinculado en ambas pantallas. No avanzar solo por un booleano de onboarding.
3. Cerrar esquema de tareas y construir backend con pruebas de autorización entre cuentas, idempotencia, versiones y borrados.
4. Extraer `HavitTask` de la vista hacia modelo/repositorio; migrar explícitamente UserDefaults por cuenta, sin inventar fechas. Conectar CRUD iOS.
5. Añadir caché/cola y sync ESP32; aplicar datos en LVGL mediante snapshots/eventos de UI. Comprobar tamaño y memoria en placa.
6. Conectar calendario, temporizador y después equipos con contratos propios.

Casos de integración necesarios: dos HAVIT cercanos con nombre corto repetido, PoP incorrecto, contraseña Wi-Fi incorrecta, falta de Internet/hora, app cerrada antes de login, publicación tardía, ACK repetido, reinicio antes/después del ACK, sesión vencida, segunda cuenta, token revocado, cambio de cuenta, evento duplicado y borrado remoto.

La recuperación `restart_claim` está escrita en iOS y firmware y documentada en la guía BLE y el contrato API. El componente de firmware y la imagen completa compilaron con ESP-IDF 5.4.2; iOS requiere Xcode en macOS y aún no se instaló. No se flasheó la placa ni se probó la recuperación en hardware.

## 10. Cómo mantener este contexto

Antes de un cambio, identificar pantalla que lo dispara, datos requeridos, endpoint, autenticación, persistencia y respuesta visual en ambos clientes. Si cambia un contrato, actualizar consumidores y este documento en la misma entrega. Registrar por separado: escrito, compilado, desplegado y probado físicamente.

Documentación de detalle: `lvgl_vscode_project/docs/VINCULACION_DISPOSITIVO.md`, `BLE_WIFI_IOS.md`, `TODAY_CALENDARIO.md`, `TAREA_RAPIDA.md`, `FLUJO_EQUIPOS.md`; `XTOOL/Havit/backend/README.md`, `XTOOL/DEVICE-API-CONTRACT.md`, `XTOOL/DEVICE-SYNC-CONTRACT.md`.

Este contexto vive en la raíz compartida, fuera de los dos repositorios hijos. Incluirlo expresamente si se comparte el workspace o se desea versionarlo.

## 11. Diagnóstico del fallo actual de PIN → Wi-Fi

El iPhone mostró que HAVIT confirmó una prueba cuyo secreto no existía en el Keychain de ese teléfono. La lectura USB mostró `network=1`, `wifi=0`, `linked=0`, así que Wi-Fi aún no se había enviado. Se agregó una recuperación local: `restart_claim` vuelve a generar únicamente la prueba pendiente sobre BLE Security 1, si aún no hay vínculo confirmado. `device_claim.c` compiló en aislamiento y como parte de la imagen completa con ESP-IDF 5.4.2. La imagen completa compiló, mide 4,243,664 bytes y deja 66 % libre en la partición app. Falta compilar iOS en macOS, instalar versiones compatibles y repetir la prueba física. El flujo iOS actual lee identidad, prepara y confirma la prueba antes de Wi-Fi; la reclamación HTTP ocurre tras autenticación.

Una cuenta puede vincular varios HAVIT y usarse desde varios iPhone. Cada unidad corresponde a una cuenta; compartirla con otras cuentas requiere membresías y permisos que aún no se implementaron.
