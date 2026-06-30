# EcoCycle — Flujo Completo

## Arquitectura

```
┌─────────────────────────────────────────────────────────────────────┐
│                        USUARIO                                      │
│  ┌──────────────────┐          ┌─────────────────────────────────┐  │
│  │  Móvil (Android) │          │  Tablet (Android)              │  │
│  │  ─────────────── │          │  ──────────────────────────────  │  │
│  │  QrActivity.kt   │          │  MainActivity.kt                │  │
│  │  · CameraX + ML  │          │  · Genera QR con sessionId      │  │
│  │    Kit Barcode    │          │  · Escucha Firebase linked      │  │
│  │  · Firebase Auth  │          │  · Real-time botellas count     │  │
│  │  · Escribe en     │          │  · Llama .NET API al finalizar  │  │
│  │    Realtime DB    │          │                                 │  │
│  └────────┬─────────┘          └────────────┬────────────────────┘  │
│           │                                 │                       │
│           │  1. Escanea QR                  │                       │
│           │  (sessionId)                    │                       │
│           │                                 │                       │
│           │  2. Escribe linked=true         │                       │
│           │     en Firebase                 │                       │
│           │                                 │                       │
│           └────────────────┬────────────────┘                       │
│                            │                                        │
│                            ▼                                        │
│              ┌─────────────────────────┐                            │
│              │  Firebase Realtime DB   │                            │
│              │  ─────────────────────   │                            │
│              │  sessions/{id}/         │                            │
│              │    linked: true         │                            │
│              │    userId: "uid"        │                            │
│              │    botellas/count: 3    │                            │
│              │    status: "activo"     │                            │
│              │                         │                            │
│              │  maquinas/{id}/         │                            │
│              │    sesion_activa: sid   │                            │
│              └───────────┬─────────────┘                            │
│                          │                                          │
│            ┌─────────────┼─────────────┐                            │
│            ▼             ▼             ▼                            │
│  ┌──────────────┐ ┌──────────┐ ┌──────────────┐                    │
│  │  Visor       │ │  YOLO    │ │  ESP32-CAM   │                    │
│  │  (NestJS)    │ │ (Python) │ │  (Arduino)   │                    │
│  │  :3000       │ │ :8000    │ │              │                    │
│  │              │ │          │ │  · WiFi      │                    │
│  │  · Consulta  │ │  · Recibe│ │  · Cámara    │                    │
│  │    sesión    │ │    imagen │ │  · IR sensor │                    │
│  │  · Envía a   │ │  · YOLOv8│ │  · 2 Servos  │                    │
│  │    YOLO      │ │  · Devuelve              │                    │
│  │  · Incrementa│ │    botella: true/false   │                    │
│  │    Firebase  │ │          │              │                    │
│  └──────┬───────┘ └──────────┘              │                    │
│         │                                   │                    │
│         ▼                                   │                    │
│  ┌────────────────┐                         │                    │
│  │  .NET Backend  │                         │                    │
│  │  :5000         │                         │                    │
│  │                │                         │                    │
│  │  POST /api/    │                         │                    │
│  │  sesionreciclaje                         │                    │
│  │                │                         │                    │
│  │  Firestore:    │                         │                    │
│  │  sesiones_     │                         │                    │
│  │  reciclaje/{id}                         │                    │
│  └────────────────┘                         │                    │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Componentes

### 1. Tablet (Maquina_EcoCycle) — `maquina_EcoCycle/`

Genera el código QR y gestiona la sesión.

**Pantallas:**
| Pantalla | Función |
|---|---|
| **Bienvenida** | Genera `sessionId = machine_001_TIMESTAMP`, muestra QR, escucha `sessions/{id}/linked` en Firebase |
| **Inicio** | Botón "INICIAR" → escribe `status=activo` en Firebase |
| **Conteo** | Escucha en tiempo real `botellas/count`, muestra contador y puntos |
| **Despedida** | Limpia sesión, llama `POST /api/sesionreciclaje` al backend .NET |

**Flujo:**
```
1. Muestra QR con sessionId
2. Escucha Firebase: cuando linked=true → escribe sesion_activa
3. Usuario presiona INICIAR → status=activo
4. Muestra conteo en tiempo real (Firebase listener)
5. Al finalizar: limpia Firebase + llama .NET API
```

**Config:** `EcoCycleConfig.kt` (generado desde `ecocycle.env`)
- `MACHINE_ID` — ID de la máquina
- `BASE_URL` — URL del backend .NET

---

### 2. Móvil (EcoCycle-Movil) — `EcoCycle-Movil/`

Escanea el QR y asocia el usuario a la sesión.

**Componentes:**
- **CameraX** — Vista previa de cámara
- **ML Kit BarcodeScanning** — Detecta y decodifica QR
- **Firebase Auth** — Obtiene usuario actual
- **Firebase Realtime DB** — Escribe `linked=true`, `userId`, `userEmail`

**Validaciones:**
- Permiso de cámara requerido
- Usuario debe estar autenticado (Firebase Auth)
- QR debe comenzar con `machine_`

**Flujo:**
```
1. Abre cámara → analiza frames con ML Kit
2. QR detectado → valida que empiece con "machine_"
3. Obtiene usuario de FirebaseAuth
4. Escribe en Firebase: sessions/{sessionId}/linked, userId, userEmail
5. Muestra "Máquina vinculada exitosamente"
```

---

### 3. Visor (NestJS) — `Visor/`

Orquesta la detección y el conteo.

**Endpoints:**

| Endpoint | Método | Descripción |
|---|---|---|
| `/detect` | POST | Detecta botella en imagen (sin sesión) |
| `/machine-detect` | POST | Detecta botella + vincula sesión activa |
| `/active-session/:machineId` | GET | Obtiene sesión activa de una máquina |

**Flujo `/machine-detect`:**
```
1. Recibe imagen + X-Machine-Id header
2. Guarda imagen en debug_images/esp32_TIMESTAMP.jpg
3. Consulta Firebase: maquinas/{machineId}/sesion_activa
4. Envía imagen a YOLO (POST /detect)
5. Si YOLO dice botella=true Y hay sesión activa:
   a. Incrementa sessions/{sessionId}/botellas/count
   b. Guarda sessions/{sessionId}/botellas/lastResult
6. Responde {botella, detected_objects, sessionId}
```

**Config:** Variables de entorno desde `ecocycle.env`
- `YOLO_HOST` / `YOLO_PORT` — Dirección del servicio YOLO
- `FIREBASE_DATABASE_URL` — URL de Firebase Realtime DB
- `FIREBASE_SERVICE_ACCOUNT_PATH` — Ruta a la clave JSON

---

### 4. YOLO (Python) — `Visor/yolo_service.py`

Servicio de detección de objetos.

**Modelo:** `yolov8s.pt` (YOLOv8 small, ~21MB, más preciso que nano)

**Clases aceptadas como botella reciclable (COCO):**

| Clase | ID | Ejemplo |
|---|---|---|
| bottle | 39 | Botella de plástico |
| wine glass | 40 | Copa/vidrio |
| cup | 41 | Taza/vaso |
| vase | 75 | Florero/envase |

**Endpoint:**
- `POST /detect` — Recibe bytes de imagen sin procesar, devuelve JSON

**Umbral de confianza:** `YOLO_CONF_THRESHOLD=0.25` (configurable)

**Preprocesamiento:**
- Aumento de contraste (1.2x)
- Aumento de nitidez (1.3x)
- Redimensionar si ancho > 1280px

---

### 5. ESP32-CAM — `CodigoArduino/`

Firmware para el microcontrolador con cámara.

**Hardware:**
| Componente | Pin | Descripción |
|---|---|---|
| Cámara OV2640 | — | Captura fotos JPEG |
| Sensor IR | GPIO 13 | Detecta objeto (INPUT_PULLUP, LOW = detectado) |
| Servo compuerta exterior | GPIO 12 | Devuelve objeto si no es botella |
| Servo compuerta interior | GPIO 14 | Deja caer botella si es reciclable |
| LED flash | GPIO 4 | Indicador visual (activo LOW) |

**Estados (modo prueba sin IR):**
```
WAITING_FOR_SESSION → CAPTURING → WAITING_FOR_SESSION
     │                    │
     ▼                    ▼
  Cada 5s:           Toma foto
  consulta Visor     Envía a Visor
  por sesión         Espera 10s
  activa             y repite
```

**LED indicador:**
- 3 parpadeos rápidos → botella detectada ✅
- 1 parpadeo largo → no es botella ❌

**Comunicación con Visor:**
```
GET  /active-session/{machineId}  → Obtiene sessionId
POST /machine-detect             → Envía foto (multipart, X-Machine-Id)
```

**Config:** `config.h` (generado desde `ecocycle.env`)
- `WIFI_SSID` / `WIFI_PASSWORD`
- `VISOR_HOST` / `VISOR_PORT`
- `MACHINE_ID`
- Pines de sensores y servos

---

### 6. Backend .NET — `EcoCycle/backend/Back/`

API REST para persistencia en Firestore.

**Endpoints:**

| Endpoint | Método | Auth | Descripción |
|---|---|---|---|
| `/api/sesionreciclaje` | POST | No | Registra sesión de reciclaje |
| `/api/sesionreciclaje/todas` | GET | JWT | Obtiene todas las sesiones |

**POST /api/sesionreciclaje:**
```json
// Request
{
  "usuarioId": "uid_123",
  "maquinaId": "machine_001",
  "botellas": 3
}

// Response
{
  "suceso": true,
  "message": "Sesión registrada correctamente.",
  "data": {
    "id": "abc123",
    "puntos": 0.3
  }
}
```

**Firestore:**
- Colección: `sesiones_reciclaje`
- Documento: `{id}` auto-generado
- Campos: `UsuarioId`, `MaquinaId`, `Botellas`, `Puntos`, `Fecha`

**Actualización de usuario:**
- Busca usuario por `UsuarioId`
- Suma `Puntos` a `SaldoPuntos`
- Guarda usuario actualizado

---

## Flujo Completo Paso a Paso

### Fase 1: Vinculación

```
Tablet                        Móvil                     Firebase
  │                             │                          │
  ├─ Genera sessionId ──────────┤                          │
  ├─ Muestra QR ───────────────→┤                          │
  │                             ├─ Escanea QR              │
  │                             ├─ Obtiene usuario Auth    │
  │                             ├─ Escribe linked=true ────→ sessions/{id}
  │                             │   userId, userEmail      │
  │                             │                          │
  ├─ Escucha sessions/{id} ←────┤                          │
  ├─ Detecta linked=true ───────┤                          │
  ├─ Escribe sesion_activa ───────────────────────────────→ maquinas/{id}
  │                                          │
  ├─ Navega a pantalla Inicio   │                          │
  │                             │                          │
  ├─ Usuario presiona INICIAR   │                          │
  ├─ Escribe status=activo ──────────────────────────────→ sessions/{id}
```

### Fase 2: Detección

```
ESP32-CAM                     Visor                      YOLO
  │                             │                          │
  ├─ Cada 5s: consulta ────────→ GET /active-session      │
  │   ¿hay sesión activa?       │                          │
  │ ←─── sessionId              │                          │
  │                             │                          │
  ├─ Toma foto (LED flash)      │                          │
  ├─ Envía foto ───────────────→ POST /machine-detect     │
  │   X-Machine-Id: machine_001 │                          │
  │                             ├─ Guarda imagen debug     │
  │                             ├─ Envía a YOLO ──────────→ POST /detect
  │                             │                          ├─ YOLOv8s infiere
  │                             │ ←── botella: true/false ─┤
  │                             │                          │
  │                             ├─ Si botella y hay sesión:
  │                             │   Incrementa count ─────→ Firebase
  │                             │   Guarda lastResult       sessions/{id}/botellas
  │                             │                          │
  │ ←── {botella, sessionId} ───┤                          │
  │                             │                          │
  ├─ ¿botella=true?             │                          │
  │   ├─ Sí: LED 3 parpadeos    │                          │
  │   │    Abre compuerta interna (GPIO 14, 3s)           │
  │   │    Cierra compuerta                               │
  │   │                                                    │
  │   ├─ No: LED 1 parpadeo     │                          │
  │        Abre compuerta externa (GPIO 12, 3s)           │
  │        Cierra compuerta                               │
```

### Fase 3: Finalización

```
Tablet                        Visor                      .NET API
  │                             │                          │
  ├─ Escucha botellas/count ←───┤ Firebase listener        │
  │   (en tiempo real)          │                          │
  │                             │                          │
  ├─ Usuario presiona FINALIZAR │                          │
  ├─ Limpia sesion_activa ──────┤ Firebase                 │
  ├─ Marca status=completado ───┤ Firebase                 │
  │                             │                          │
  ├─ POST /api/sesionreciclaje ──────────────────────────→ .NET Backend
  │   {maquinaId, usuarioId,    │                          │
  │    botellas}                │                          │
  │                             │                          ├─ Guarda en Firestore
  │                             │                          ├─ Actualiza SaldoPuntos
  │ ←── {id, puntos} ←──────────┤                          │
  │                             │                          │
  ├─ Muestra "Gracias" con      │                          │
  │   total de puntos            │                          │
```

---

## Firebase Estructura

### Realtime Database (IoT / tiempo real)

```
/
├── sessions/
│   └── {sessionId}/
│       ├── linked: true              ← Móvil escribe
│       ├── userId: "uid_123"         ← Móvil escribe
│       ├── userEmail: "a@b.com"      ← Móvil escribe
│       ├── linkedAt: 123456789       ← Móvil escribe
│       ├── status: "activo"          ← Tablet escribe
│       │           "completado"
│       └── botellas/
│           ├── count: 3              ← Visor incrementa
│           └── lastResult/
│               ├── botella: true     ← Visor escribe
│               └── timestamp: ...    ← Visor escribe
│
└── maquinas/
    └── {machineId}/
        └── sesion_activa: "{sessionId}"  ← Tablet escribe
```

### Firestore (Persistencia / permanente)

```
sesiones_reciclaje/           ← Colección
  └── {docId}/                ← Documento auto-generado
      ├── UsuarioId: "uid_123"
      ├── MaquinaId: "machine_001"
      ├── Botellas: 3
      ├── Puntos: 0.3
      └── Fecha: Timestamp
```

**Reglas de seguridad Realtime Database (desarrollo):**
```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

---

## Configuración Centralizada

**Archivo único:** `ecocycle.env`

```env
# --- Máquina ---
MACHINE_ID=machine_001

# --- Visor (NestJS) ---
VISOR_PORT=3000

# --- YOLO ---
YOLO_PORT=8000
YOLO_CONF_THRESHOLD=0.25
YOLO_MODEL_PATH=yolov8s.pt

# --- Firebase ---
FIREBASE_DATABASE_URL=https://ecocycle-e9c04-default-rtdb.firebaseio.com
FIREBASE_PROJECT_ID=ecocycle-e9c04

# --- .NET Backend ---
NET_API_PORT=5000

# --- WiFi (ESP32-CAM) ---
WIFI_SSID=Totalplay-C8B1
WIFI_PASSWORD=C8B1CBB8khASQe6W

# --- ESP32 Pines ---
IR_SENSOR_PIN=13
OUTER_GATE_PIN=12
INNER_GATE_PIN=14

# --- Red ---
SERVER_HOST=192.168.100.19
```

**Regenerar configs después de cambios:**
```bash
./run.sh genconfig
```

Esto genera:
- `CodigoArduino/config.h` → Para el ESP32-CAM
- `EcoCycleConfig.kt` → Para las apps Android (tablet y móvil)

---

## Comandos Útiles

### Servicios
```bash
./run.sh all              # Inicia Visor + EcoCycle
./run.sh visor            # Solo Visor (YOLO + NestJS)
./run.sh ecocycle         # Solo .NET + Angular
./run.sh stop             # Detiene todo
```

### Simulación / Pruebas
```bash
./run.sh flow              # QR → 2 botellas → puntos (limpia)
./run.sh flow foto.jpg 5   # QR → 5 botellas con imagen custom
./run.sh simulate --botellas 3 --image x.jpg   # Control total
```

### Configuración
```bash
./run.sh genconfig         # Regenera configs desde ecocycle.env
./run.sh setup-fb          # Crea placeholders Firebase
```

### Verificación manual
```bash
# Probar detección YOLO
curl -X POST http://localhost:3000/detect -F "image=@foto.jpg"

# Crear sesión de prueba
curl -X PUT "https://ecocycle-e9c04-default-rtdb.firebaseio.com/sessions/test_123.json" \
  -H "Content-Type: application/json" \
  -d '{"linked":true,"userId":"test","status":"vinculado"}'

# Activar máquina
curl -X PUT "https://ecocycle-e9c04-default-rtdb.firebaseio.com/maquinas/machine_001/sesion_activa.json" \
  -H "Content-Type: application/json" \
  -d '"test_123"'

# Ver conteo
curl -s "https://ecocycle-e9c04-default-rtdb.firebaseio.com/sessions/test_123/botellas/count.json"
```

---

## Solución de Problemas

### ESP32-CAM no conecta WiFi
- Verificar `WIFI_SSID` y `WIFI_PASSWORD` en `ecocycle.env`
- Regenerar configs: `./run.sh genconfig`
- Re-subir firmware

### Cámara falla (error 0xffffffff)
- Activar PSRAM en Arduino IDE: Tools → PSRAM → Enabled
- Usar resolución VGA o QVGA
- Verificar fuente de poder (mínimo 5V 2A)

### YOLO no detecta botella
- El ESP32-CAM debe apuntar directamente a la botella (15-20cm)
- Buena iluminación
- Usar `yolov8s.pt` (no `yolov8n.pt`) para mejor precisión
- Bajar umbral: `YOLO_CONF_THRESHOLD=0.1` en `ecocycle.env`

### Servos no se mueven
- **Causa más común:** alimentación insuficiente
- Los servos NECESITAN fuente externa de 5V (no solo el USB del Arduino)
- Usar power bank o fuente de 5V 2A
- Conectar tierra (GND) común entre ESP32 y fuente de servos

### Backend .NET no arranca
- Falta la clave de Firebase: descargar de Firebase Console
- Guardar en: `EcoCycle/backend/Back/firebase-key.json`
- También en: `Visor/firebase-service-account.json`
- Es el mismo archivo, copiar a ambas ubicaciones

### Firestore sin registros
- El backend .NET debe estar funcionando
- El POST a `/api/sesionreciclaje` no requiere autenticación
- Verificar logs: `docker logs ecocycle_backend`

---

## Puertos

| Puerto | Servicio | Descripción |
|---|---|---|
| 3000 | Visor (NestJS) | API de detección y sesiones |
| 8000 | YOLO (Python) | Servicio de inferencia |
| 5000 | .NET Backend | API REST + Firestore |
| 4200 | Angular Frontend | Interfaz web |
