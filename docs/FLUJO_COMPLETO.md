# EcoCycle — Flujo Completo

## Arquitectura General

```
┌────────────────────────────────────────────────────────────────────────────┐
│                           INTERNET                                         │
│                                                                            │
│  ┌──────────────────┐       ┌──────────────────┐       ┌────────────────┐ │
│  │  Tablet (Android) │       │  Móvil (Android)  │       │  Web Admin     │ │
│  │  maquina_EcoCycle │       │  EcoCycle-Movil   │       │  (Angular 19)  │ │
│  │                   │       │                   │       │                │ │
│  │  · CameraX (1280) │       │  · ML Kit QR      │       │  · Dashboard   │ │
│  │  · Envía frames   │       │  · Firebase Auth  │       │  · Usuarios    │ │
│  │  · Muestra QR     │       │  · Escanea QR     │       │  · Reportes    │ │
│  │  · Conteo tiempo  │       │  · Vincula sesión │       │  · Recompensas │ │
│  │    real           │       │                   │       │                │ │
│  └────────┬──────────┘       └────────┬──────────┘       └───────┬────────┘ │
│           │                          │                           │          │
│           │ POST /detect             │ Escribe linked            │          │
│           │ POST /machine-validate   │ en Firebase               │          │
│           │                          │                           │          │
│           ▼                          ▼                           ▼          │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                         VPS (104.248.187.43)                        │  │
│  │  ┌────────────────────────────────────────────────────────────────┐  │  │
│  │  │  Docker - Visor                                               │  │  │
│  │  │  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐   │  │  │
│  │  │  │  NestJS       │────▶│  YOLO Python │     │  Firestore   │   │  │  │
│  │  │  │  :3000        │     │  :8000       │     │  (Admin)     │   │  │  │
│  │  │  │  argus_backend│     │  argus_yolo  │     └──────────────┘   │  │  │
│  │  │  │               │     │  yolo11n.pt  │                        │  │  │
│  │  │  │  · /detect    │     │  yolo11n.pt  │                        │  │  │
│  │  │  │  · /machine-* │     │  conf=0.1    │                        │  │  │
│  │  │  │  · /gate-*    │     │              │                        │  │  │
│  │  │  └──────┬───────┘     └──────────────┘                        │  │  │
│  │  │         │                                                      │  │  │
│  │  │         ▼                                                      │  │  │
│  │  │  ┌────────────────────────────────────────────────┐            │  │  │
│  │  │  │  Firebase Realtime Database                     │            │  │  │
│  │  │  │  ────────────────────────────────               │            │  │  │
│  │  │  │  sessions/{id}/linked, userId,                  │            │  │  │
│  │  │  │  botellas/count, validacion1, gate_command       │            │  │  │
│  │  │  │  maquinas/{id}/sesion_activa                     │            │  │  │
│  │  │  └────────────────────────────────────────────────┘            │  │  │
│  │  └────────────────────────────────────────────────────────────────┘  │  │
│  │                                                                      │  │
│  │  ┌────────────────────────────────────────────────────────────────┐  │  │
│  │  │  Docker - EcoCycle (opcional)                                  │  │  │
│  │  │  ┌──────────────┐     ┌──────────────┐                        │  │  │
│  │  │  │  .NET 8      │     │  Angular 19  │                        │  │  │
│  │  │  │  :5000       │     │  :4200       │                        │  │  │
│  │  │  │  API REST    │     │  Admin panel  │                        │  │  │
│  │  │  └──────────────┘     └──────────────┘                        │  │  │
│  │  └────────────────────────────────────────────────────────────────┘  │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                            │
│  ┌──────────────────┐                                                      │
│  │  ESP32 (Arduino) │                                                      │
│  │  ──────────────  │                                                      │
│  │  · WiFi          │                                                      │
│  │  · 2 Servos:     │                                                      │
│  │    - Exterior    │                                                      │
│  │    - Interior    │                                                      │
│  │  · Polling cada  │                                                      │
│  │    2s GET /gate  │                                                      │
│  │    -command/{id} │                                                      │
│  └──────────────────┘                                                      │
└────────────────────────────────────────────────────────────────────────────┘
```

---

## Componentes

### 1. Tablet (Máquina EcoCycle) — `maquina_EcoCycle/`

App Android Jetpack Compose. Es la interfaz principal para el usuario en la máquina.

**Pantallas (5):**

| Pantalla | Función |
|---|---|
| **Bienvenida** | Genera `sessionId = machine_001_TIMESTAMP`, muestra QR con ZXing, escucha `sessions/{id}/linked` en Firebase Realtime |
| **Inicio** | Botón "INICIAR" → escribe `status=activo` en Firebase |
| **Validación** | Cámara CameraX (1280x720, YUV→JPEG), envía frames a Visor cada 300ms (cooldown 1.5s entre detecciones) |
| **Conteo** | Escucha en tiempo real `botellas/count`, muestra contador y puntos acumulados |
| **Despedida** | Limpia sesión en Firebase, llama `POST /api/sesionreciclaje` al backend .NET |

**Detección en bucle:**
```
ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST
Cada 300ms → captura frame → YUV a JPEG → POST /detect a Visor
Si botella == true → POST /machine-validate → escribe gate_command en Firebase
Si botella == false → reintenta (mínimo 1.5s entre detecciones)
```

**Network:** OkHttp, 5s connect / 10s read timeout

---

### 2. Móvil (EcoCycle-Movil) — `EcoCycle-Movil/`

App Android para usuarios finales. Escanea QR y gestiona sesión.

**Componentes:**
- **CameraX + ML Kit BarcodeScanning** — Detecta y decodifica QR
- **Firebase Auth** — Auth por email/password
- **Firebase Realtime DB** — Escribe linked=true, userId, userEmail
- **Retrofit** — Consume .NET API para perfil, historial, recompensas

**Flujo:**
```
1. Abre cámara → analiza frames con ML Kit
2. QR detectado → valida que empiece con "machine_"
3. Obtiene usuario de FirebaseAuth
4. Escribe en Firebase: sessions/{sessionId}/linked, userId, userEmail
5. Navega a menú principal
```

---

### 3. Visor (NestJS) — `Visor/`

Backend orquestador. Corre en Docker como `argus_backend`.

**Endpoints:**

| Endpoint | Método | Descripción |
|---|---|---|
| `/detect` | POST | Recibe imagen (multipart field "image") → reenvía a YOLO → devuelve `{botella, detected_objects}` |
| `/machine-detect` | POST | Como `/detect` pero vincula con sesión activa (header `X-Machine-Id`). Si botella=true y hay sesión, incrementa contador en Firebase |
| `/active-session/:machineId` | GET | Obtiene `sesion_activa` para una máquina |
| `/machine-validate` | POST | Escribe `validacion1` y `gate_command` en Firebase (tableta confirma detección) |
| `/gate-command/:machineId` | GET | Obtiene y elimina `gate_command` de Firebase (consumo único por ESP32) |
| `/machine-confirm` | POST | Escribe `validacion2` en Firebase (ESP32 confirma segunda validación) |
| `/machine-cleanup/:machineId` | POST | Limpia `sesion_activa` y `gate_command` de Firebase |
| `/session-status/:sessionId` | GET | Obtiene estado de sesión desde Firebase |

**Dependencias:**
- `YOLO_HOST` / `YOLO_PORT` — Dirección del servicio YOLO (docker compose: `yolo:8000`)
- `FIREBASE_DATABASE_URL` — URL de Firebase Realtime DB
- `FIREBASE_SERVICE_ACCOUNT_PATH` — Ruta a clave JSON de servicio

**FirebaseService:** Implementa REST HTTP directo a Firebase Realtime Database (sin SDK). Lee/escribe paths como `/sessions/{id}/...` y `/maquinas/{id}/...`.

---

### 4. YOLO (Python) — `Visor/yolo_service.py`

Servicio HTTP de detección de objetos. Corre en Docker como `argus_yolo`.

**Configuración actual:**

| Parámetro | Valor | Descripción |
|---|---|---|
| Puerto | `8000` | Servidor HTTP |
| Modelo | `yolo11n.pt` (nano) | YOLO11 nano (5.4MB, rápido, preciso) |
| Confianza mínima | `0.1` | Configurable via env `YOLO_CONF_THRESHOLD` |
| Clases botella | `{39, 40, 41}` | bottle(39), wine glass(40), cup(41) |

**Endpoint:** `POST /detect` — Recibe bytes de imagen (application/octet-stream) → devuelve JSON:
```json
{
  "botella": true,
  "detected_objects": [
    {"name": "bottle", "confidence": 0.87}
  ]
}
```

**Preprocesamiento adaptativo:**
```
Entrada: imagen RGB (hasta 3000x3000)
1. Si lado mayor > 1280px → redimensionar manteniendo aspect ratio
2. Calcular brillo promedio:
   - Si brillo < 50 (oscura) → aplicar corrección gamma + autocontrast + nitidez
   - Si brillo >= 50 (normal) → brillo 1.1x + nitidez 1.2x
3. Convertir a numpy array y pasar a YOLO
```

**Diagnóstico de imágenes oscuras:**
La tablet envía imágenes 3000x3000 JPEG. Se descubrió que las fotos tomadas dentro de la máquina (sin luz) tienen brillo promedio de ~3.5/255 (casi negro). El preprocessing con gamma=0.15 + autocontrast logra llevarlo a ~128/255, pero YOLO solo detecta ruido (confianza <10%) porque la imagen original no contiene información utilizable. **Se requiere iluminación dentro de la máquina.**

**Debug:** Cada imagen recibida se guarda en `debug_images/yolo_{timestamp}.jpg` dentro del contenedor.

---

### 5. ESP32 (Arduino) — `CodigoArduino/`

Firmware para ESP32 con control de compuertas (sin cámara, sin segunda validación).

**Hardware:**

| Componente | Pin | Descripción |
|---|---|---|
| Servo compuerta exterior | GPIO 12 | Abre para recibir o devolver botella |
| Servo compuerta interior | GPIO 14 | Abre para almacenar botella |
| (IR sensor no usado) | GPIO 13 | Previsto pero no implementado |

**Firmware actual (v5 - Solo compuerta):**
```
Loop principal (cada 2s):
  GET /gate-command/{MACHINE_ID} → http://VISOR_HOST:3000
  Si respuesta contiene "openOuter: true":
    Servo exterior → 90° (3s) → 0°
    (Sin cámara, sin segunda validación → versión simplificada)
```

**Config:** `config.h` (auto-generado desde `ecocycle.env`):
```cpp
#define WIFI_SSID "Redmi 13"
#define WIFI_PASSWORD "urielzavaal"
#define VISOR_HOST "104.248.187.43"
#define VISOR_PORT 3000
#define MACHINE_ID "machine_001"
#define OUTER_GATE_PIN 12
#define INNER_GATE_PIN 14
```

---

### 6. Backend .NET 8 — `EcoCycle/backend/Back/`

API REST para persistencia de usuarios y sesiones.

**Tecnologías:** ASP.NET Core 8, Firestore SDK, JWT (Firebase Auth), FluentValidation, Serilog

**Colecciones en Firestore:**
- `usuarios` — Perfiles de usuario (nombre, email, saldoPuntos, rol)
- `sesiones_reciclaje` — Sesiones completadas (usuarioId, maquinaId, botellas, puntos, fecha)
- `recompensas` — Catálogo de recompensas (nombre, costoPuntos, stock)
- `canjes` — Canjes realizados por usuarios
- `proveedores` / `compra_proveedores` / `materia_prima` — Gestión de proveedores e inventario
- `notificaciones` / `comentarios` — Comunicación con usuarios

**Endpoint clave:**
```
POST /api/sesionreciclaje
{ "usuarioId": "uid_123", "maquinaId": "machine_001", "botellas": 3 }
→ { "suceso": true, "data": { "id": "...", "puntos": 60 } }
```

---

### 7. Panel Web Angular 19 — `EcoCycle/frontend/front/`

Panel administrativo standalone (Bootstrap 5, Chart.js, SweetAlert2, jsPDF).

**Rutas principales:**
- `/login`, `/registro` — Auth
- `/cliente/*` — Dashboard, historial, recompensas, perfil
- `/admin/*` — Dashboard global, usuarios, sesiones, catálogo, proveedores, reportes

---

## Firebase Estructura

### Realtime Database (control en tiempo real)

```
/
├── sessions/
│   └── {sessionId}/
│       ├── linked: true                    ← Móvil escribe
│       ├── userId: "uid_123"               ← Móvil escribe
│       ├── userEmail: "a@b.com"            ← Móvil escribe
│       ├── linkedAt: timestamp             ← Móvil escribe
│       ├── status: "vinculado"|"activo"|"completado"  ← Tablet
│       ├── botellas/
│       │   ├── count: 3                    ← Visor incrementa
│       │   └── lastResult: { botella, timestamp }
│       ├── validacion1: { esBotella, machineId, timestamp }  ← Visor
│       └── validacion2: { esBotella, machineId, timestamp }  ← Visor
│
└── maquinas/
    └── {machineId}/
        ├── sesion_activa: "{sessionId}"    ← Tablet escribe
        └── gate_command: { openOuter, sessionId } | null  ← Visor escribe/borra
```

### Firestore (persistencia)

```
sesiones_reciclaje/{docId}
  usuarioId, maquinaId, botellas, puntos, fecha

usuarios/{uid}
  nombre, email, saldoPuntos, rol, activo, ...

recompensas/{id}
  nombre, costoPuntos, stock, activa, ...

canjes/{id}
  usuarioId, recompensaId, puntosUsados, fecha
```

---

## Flujo Completo Paso a Paso

### Fase 1: Vinculación Usuario-Máquina

```
Tablet                        Móvil                     Firebase
  │                             │                          │
  ├─ Genera sessionId ──────────┤                          │
  ├─ Muestra QR ───────────────→┤                          │
  │                             ├─ Escanea QR (ML Kit)     │
  │                             ├─ Obtiene usuario Auth    │
  │                             ├─ Escribe linked=true ────→ sessions/{id}
  │                             │   userId, userEmail      │
  │                             │                          │
  ├─ Escucha sessions/{id} ←────┤                          │
  │     (Firebase listener)     │                          │
  ├─ Detecta linked=true ───────┤                          │
  ├─ Escribe sesion_activa ───────────────────────────────→ maquinas/{id}
  │                                          │
  ├─ Navega a pantalla Inicio   │                          │
  │                             │                          │
  ├─ Usuario presiona INICIAR   │                          │
  ├─ Escribe status=activo ──────────────────────────────→ sessions/{id}
```

### Fase 2: Detección Primaria (Tableta + YOLO)

```
Tablet                        Visor                      YOLO
  │                             │                          │
  ├─ Inicia cámara CameraX      │                          │
  │   (1280x720, YUV→JPEG)      │                          │
  │                             │                          │
  ├─ Bucle cada 300ms:          │                          │
  │   Toma frame                │                          │
  │   POST /detect ────────────→ (reenvía a YOLO) ────────→ POST /detect
  │   (multipart, field "image")│                          │
  │                             │                          ├─ Preprocessing:
  │                             │                          │   Si oscura → gamma+autocontrast
  │                             │                          │   Si normal → brillo+nitidez
  │                             │                          ├─ Inferencia yolo11n
  │                             │                          │   conf ≥ 0.1, clase 39/40/41
  │                             │ ←── {botella, objetos} ──┤
  │ ←── {botella, objetos} ─────┤                          │
  │                             │                          │
  ├─ ¿botella==true?            │                          │
  │   ├─ Sí:                    │                          │
  │   │   Borde verde           │                          │
  │   │   Espera 1.5s cooldown  │                          │
  │   │   POST /machine-validate ──────────────────────────→ Firebase
  │   │     {sessionId,         │        Escribe gate_command
  │   │      machineId,         │        en maquinas/{id}/
  │   │      esBotella: true}   │
  │   │                         │                          │
  │   └─ No:                    │                          │
  │       Borde rojo            │                          │
  │       Sigue capturando      │                          │
```

### Fase 3: Acción Física (ESP32 + Compuertas)

```
ESP32                         Visor                      Firebase
  │                             │                          │
  ├─ Poll cada 2s:              │                          │
  │   GET /gate-command/{id} ───→                          │
  │                             ├─ Lee gate_command ───────→ maquinas/{id}/
  │                             │   (y lo borra: consumo único)
  │ ←── {openOuter, sessionId} ─┤                          │
  │                             │                          │
  ├─ ¿openOuter==true?          │                          │
  │   ├─ Sí:                    │                          │
  │   │   Servo exterior 90°    │                          │
  │   │   Espera 3s             │                          │
  │   │   Servo exterior 0°     │                          │
  │   │                         │                          │
  │   │   POST /machine-confirm ───────────────────────────→ Firebase
  │   │     {sessionId,         │        Escribe validacion2
  │   │      machineId,         │
  │   │      esBotella: true}   │
  │   │                         │                          │
  │   └─ No:                    │                          │
  │       Sigue polling         │                          │
```

### Fase 4: Finalización

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
  │                             │                          ├─ Suma puntos (botellas*20)
  │ ←── {id, puntos} ←──────────┤                          │
  │                             │                          │
  ├─ Muestra "Gracias" con      │                          │
  │   total de puntos            │                          │
```

---

## Configuración Centralizada

**Archivo único:** `ecocycle.env` (raíz del proyecto)

```env
# --- Máquina ---
MACHINE_ID=machine_001

# --- Visor (NestJS) ---
VISOR_PORT=3000

# --- YOLO ---
YOLO_PORT=8000
YOLO_CONF_THRESHOLD=0.1
YOLO_MODEL_PATH=yolo11n.pt

# --- Firebase ---
FIREBASE_DATABASE_URL=https://ecocycle-e9c04-default-rtdb.firebaseio.com
FIREBASE_PROJECT_ID=ecocycle-e9c04

# --- .NET Backend ---
NET_API_PORT=5000

# --- WiFi (ESP32) ---
WIFI_SSID=Redmi 13
WIFI_PASSWORD=urielzavaal

# --- ESP32 Pines ---
IR_SENSOR_PIN=13
OUTER_GATE_PIN=12
INNER_GATE_PIN=14

# --- Red ---
SERVER_HOST=104.248.187.43
SERVER_DOMAIN=ecocyclemx.tech
```

**Regenerar configs:**
```bash
python3 scripts/generate_configs.py
# o
./run.sh genconfig
```

Esto genera:
- `CodigoArduino/config.h` → Para el ESP32
- `maquina_EcoCycle/.../EcoCycleConfig.kt` → Tablet
- `EcoCycle-Movil/.../EcoCycleConfig.kt` → Móvil

---

## Puertos

| Puerto | Servicio | Contenedor | Descripción |
|---|---|---|---|
| 3000 | Visor (NestJS) | `argus_backend` | API de detección y sesiones |
| 8000 | YOLO (Python) | `argus_yolo` | Servicio de inferencia |
| 5000 | .NET Backend | `ecocycle_backend` | API REST + Firestore |
| 4200 | Angular Frontend | `ecocycle_frontend` | Panel admin web |

---

## Comandos Útiles

### Servicios
```bash
./run.sh all              # Inicia Visor + EcoCycle
./run.sh visor            # Solo Visor (YOLO + NestJS)
./run.sh ecocycle         # Solo .NET + Angular
./run.sh stop             # Detiene todo
```

### YOLO (build y deploy)
```bash
cd Visor
docker-compose build yolo          # Reconstruir imagen
docker-compose up -d yolo          # Iniciar
docker rm -f argus_yolo            # Forzar recreación
docker logs argus_yolo             # Ver logs
```

### Pruebas
```bash
# Detección directa a YOLO
curl -X POST http://localhost:8000/detect \
  --data-binary @foto.jpg \
  -H 'Content-Type: application/octet-stream'

# Detección vía Visor
curl -X POST http://localhost:3000/detect \
  -F "image=@foto.jpg"

# Verificar estado
curl http://localhost:3000/active-session/machine_001
curl http://localhost:3000/gate-command/machine_001
```

### Depuración YOLO
```bash
# Ver confianza dentro del contenedor
docker exec argus_yolo python3 -c "import os; print(os.environ.get('YOLO_CONF_THRESHOLD'))"

# Ver clases aceptadas
docker exec argus_yolo python3 -c "
from ultralytics import YOLO
m = YOLO('yolo11n.pt')
print([m.names[c] for c in [39, 40, 41]])
"
```

---

## Diagnóstico de Problemas Conocidos

### Imágenes oscuras (sin detección)
**Síntoma:** La cámara de la tablet toma fotos pero YOLO no detecta nada.
**Causa:** El interior de la máquina no tiene iluminación. El brillo promedio de las imágenes es ~3.5/255 (casi negro). Incluso con corrección gamma agresiva, no hay suficiente detalle para que YOLO reconozca objetos.
**Solución:** Agregar LED de iluminación dentro de la máquina apuntando al área donde se coloca la botella.

### YOLO no detecta botella (con luz)
**Síntoma:** Con buena iluminación, a veces no detecta.
**Causa:** Confianza demasiado alta o clase no incluida.
**Solución:** Verificar `YOLO_CONF_THRESHOLD` (actual 0.1) y `BOTTLE_CLASSES={39,40,41}`. Probar con `conf=0.05` temporalmente.

### ESP32 no abre compuerta
**Síntoma:** La tablet confirma detección pero el servo no se mueve.
**Causa:** Alimentación insuficiente para los servos.
**Solución:** Usar fuente externa de 5V 2A para los servos, con tierra común con el ESP32.

### Visor no conecta con YOLO
**Síntoma:** Backend logs muestran `ECONNREFUSED 172.19.0.2:8000`
**Causa:** YOLO no está corriendo o se reinició.
**Solución:** `docker ps | grep argus_yolo` → si no aparece: `docker-compose up -d yolo`

---

## Historial de Cambios Recientes

| Fecha | Cambio |
|---|---|
| 2026-07-08 | YOLO: confianza bajada a 0.1, clases ampliadas a {39,40,41} |
| 2026-07-08 | YOLO: preprocessing adaptativo con gamma para imágenes oscuras |
| 2026-07-08 | YOLO: modelo cambiado a yolo11n.pt (más rápido que yolov8s) |
| 2026-07-08 | ESP32: firmware v5 simplificado a solo compuerta (sin cámara) |
| 2026-07-08 | Diagnóstico: identificado que la máquina necesita iluminación LED |
