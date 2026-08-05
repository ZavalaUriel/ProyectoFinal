# EcoCycle ♻️

**Sistema inteligente de reciclaje de botellas PET**

Máquina IoT con visión artificial que valida botellas de plástico automáticamente,
acredita **puntos canjeables** a los usuarios y entrega datos de reciclaje en tiempo
real a los administradores. Proyecto integrador — "Recicla + Gana".

> ⚠️ Repositorio **multi-repo**: la raíz es un repo git; `EcoCycle`, `EcoCycle-Movil`,
> `Visor` y `maquina_EcoCycle` son repos git anidados (tratados como submódulos sin
> `.gitmodules`). Cada commit va al repo de su carpeta.

---

## Tabla de contenidos

1. [Descripción general](#1-descripción-general)
2. [Arquitectura](#2-arquitectura)
3. [Componentes y stack tecnológico](#3-componentes-y-stack-tecnológico)
4. [Estructura del repositorio](#4-estructura-del-repositorio)
5. [Flujo end-to-end de la máquina](#5-flujo-end-to-end-de-la-máquina)
6. [Configuración centralizada (`ecocycle.env`)](#6-configuración-centralizada)
7. [Backend .NET (API REST)](#7-backend-net-api-rest)
8. [Visor NestJS (detección y orquestación)](#8-visor-nestjs)
9. [YOLO (microservicio Python)](#9-yolo-microservicio-python)
10. [Frontend Angular](#10-frontend-angular)
11. [Firmware ESP32](#11-firmware-esp32)
12. [Apps Android](#12-apps-android)
13. [Correr en local (desarrollo)](#13-correr-en-local-desarrollo)
14. [Despliegue en el VPS (producción)](#14-despliegue-en-el-vps-producción)
15. [CI/CD (GitHub Actions)](#15-cicd-github-actions)
16. [Seguridad](#16-seguridad)
17. [Scripts y orquestación](#17-scripts-y-orquestación)
18. [Docker Compose](#18-docker-compose)
19. [Verificación y pruebas](#19-verificación-y-pruebas)
20. [Solución de problemas](#20-solución-de-problemas)
21. [Documentación adicional](#21-documentación-adicional)

---

## 1. Descripción general

EcoCycle integra un **prototipo físico IoT** (máquina recicladora) con una **app móvil**,
una **tableta interactiva**, una **plataforma web** y servicios en la nube:

1. El usuario escanea un **código QR** en la tableta de la máquina con su teléfono.
2. Coloca una **botella PET** frente a la cámara de la tableta.
3. El sistema detecta automáticamente si es una botella válida con **visión artificial**
   (Gemini → fallback YOLO).
4. Si es válida, el **ESP32** abre la compuerta, la botella se almacena y se
   **acreditan puntos** al usuario (20 pts por botella).
5. Los puntos son canjeables por **recompensas** en el catálogo.

**Beneficios:**
- Incentiva el reciclaje mediante un sistema gamificado de puntos.
- Elimina la clasificación y el conteo manual.
- Doble validación (cámara de tableta + microservicio de visión) para reducir errores.
- Datos en tiempo real para administradores (dashboard con gráficas Chart.js).

**Alcance:** solo botellas PET. **Excluye:** otros materiales, pagos económicos,
notificaciones push, soporte iOS y multi-idioma.

---

## 2. Arquitectura

```
                    ┌─────────────────────────────┐
                    │       VPS (Ubuntu)          │
                    │                             │
   Usuario          │   ┌─────────┐  ┌─────────┐  │
 ┌───────────┐      │   │ YOLO py │  │  Visor  │  │
 │ App móvil │─────▶│   │  :8000  │◀─│ NestJS  │  │
 │ (Android) │      │   └─────────┘  │  :3000  │  │
 └─────┬─────┘      │        ▲       └────┬────┘  │
       │ QR + API   │        │ octet      │ REST   │
       ▼            │   ┌────┴─────┐  ┌────┴────┐  │
 ┌───────────┐      │   │ Firebase │  │ .NET    │  │
 │  Tableta  │─────▶│   │  RTDB +  │◀─│ Backend │  │
 │ Android   │      │   │ Firestore│  │  :5000  │  │
 └─────┬─────┘      │   └────┬─────┘  └────┬────┘  │
       │           ┌─┴───────┴─────┐  ┌────┴────┐  │
       │           │     Nginx     │  │ Angular │  │
       │           │  :80 / :443   │──▶│   SPA   │  │
       │           └───────────────┘  │  :4200  │  │
       │                              └─────────┘  │
       │                                            │
       ▼                                            │
 ┌───────────┐    WiFi + polling /gate-command      │
 │  ESP32-CAM│◀─────────────────────────────────────┘
 │  + servos │
 └───────────┘
```

**Comunicación entre componentes (API REST/HTTPS + Firebase):**

| De → A | Medio | Protocolo |
|---|---|---|
| App móvil → .NET | API REST | HTTPS con `Authorization: Bearer` (token Firebase) |
| App móvil → Firebase Firestore | SDK | Vinculación de sesión vía QR |
| Tableta → Visor | API REST | Multipart (frames JPEG) + `X-Api-Key` |
| Tableta → .NET | API REST | `POST /api/SesionReciclaje` al finalizar |
| Visor → YOLO | HTTP | bytes de imagen (`application/octet-stream`) |
| Visor → Firebase RTDB | REST con OAuth2 | `gate_command`, `sesion_activa`, `validacion*` |
| ESP32 → Visor | HTTP GET/POST | Polling `/gate-command/{id}` cada 2 s + `X-Api-Key` |
| Angular → .NET | API REST | Proxy nginx `/api/` |

---

## 3. Componentes y stack tecnológico

| Componente | Carpeta | Tecnología | Rol |
|---|---|---|---|
| Backend API | `EcoCycle/backend/Back` | **.NET 8** + Firebase Firestore | Usuarios, puntos, recompensas, catálogo, producción, reportes, dashboard |
| Frontend web | `EcoCycle/frontend/front` | **Angular 19** + Firebase Auth | Panel de clientes y administradores |
| Visor | `Visor/src` | **NestJS 11** | Detección (Gemini → YOLO) y orquestación de la máquina |
| YOLO service | `Visor/yolo_service.py` | **Python** (`http.server` puro + ultralytics) | Detección de respaldo (clase COCO `bottle`) |
| App máquina | `maquina_EcoCycle` | **Android Kotlin** (Jetpack Compose) | Tablet de la máquina recicladora |
| App móvil | `EcoCycle-Movil` | **Android Kotlin** (Jetpack Compose) | App del usuario final (QR, puntos, canjes) |
| Firmware | `CodigoArduino` | **Arduino/ESP32-CAM** | Compuerta física (servo exterior) |
| Infraestructura | — | **Docker**, **Nginx**, **DigitalOcean VPS** | Contenerización, proxy inverso y TLS |

### Versiones clave (verificadas en `package.json` / `.csproj` / `build.gradle`)

| Dependencia | Versión |
|---|---|
| Angular / @angular/core | 19.2.0 (CLI 19.2.17, TypeScript ~5.7) |
| NestJS | 11.0.1 |
| .NET | 8.0 |
| Firebase (JS) | 11.10.0 (+ `@angular/fire` 19.2.0) |
| chart.js | 4.5.1 |
| bootstrap | 5.3.8 |
| sweetalert2 | 11.26.25 |
| exceljs / xlsx / jspdf | 4.4.0 / 0.18.5 / 4.2.1 |
| Google.Cloud.Firestore | 4.3.0 |
| BCrypt.Net-Next | 4.2.0 |
| firebase-admin (Visor) | 12.7.0 |
| ultralytics (YOLO) | — (precarga modelo `yolo11n.pt`) |
| Kotlin / AGP (máquina) | 2.2.10 / 9.2.1 |
| Kotlin / AGP (móvil) | 2.3.0 / 8.13.2 |

---

## 4. Estructura del repositorio

```
ProyectoFinal/
├── EcoCycle/                  ← repo git anidado (SuprSalva/EcoCycle)
│   ├── backend/
│   │   ├── Back/              # API .NET 8 (Controllers → Services → Repositories)
│   │   │   ├── Controllers/   # 17 controllers
│   │   │   ├── Repositories/  # Patrón repositorio sobre Firestore
│   │   │   ├── Services/      # FirebaseAuth, Email, PDF, Productos, Producción
│   │   │   ├── Models/        # DTOs
│   │   │   ├── Auth/          # AdminOnlyAttribute, MachineApiKeyAttribute
│   │   │   └── firebase-key.json   # (NO versionado) clave de servicio
│   │   └── Dockerfile         # imagen multi-stage (sdk → aspnet)
│   ├── frontend/
│   │   ├── front/             # SPA Angular 19
│   │   │   ├── src/app/features/    # 32 módulos (login, dashboard-*, productos…)
│   │   │   ├── src/app/core/        # guards, interceptors, services
│   │   │   ├── src/app/shared/      # navbar, sidebar, loader, directivas
│   │   │   ├── src/app/models/      # modelos de datos
│   │   │   └── angular.json         # budgets de build (25kb/50kb estilos)
│   │   └── Dockerfile         # multi-stage (node:20 build → nginx serve)
│   ├── .github/workflows/deploy.yml # CI/CD → VPS
│   └── docker-compose.yml     # compose de DESARROLLO (hot-reload)
├── Visor/                     ← repo git anidado (ZavalaUriel/Visor)
│   ├── src/                   # NestJS (app.controller/service, gemini, firebase, guard)
│   ├── yolo_service.py        # microservicio YOLO (puerto 8000)
│   ├── Dockerfile.nestjs      # imagen Visor
│   ├── Dockerfile.yolo        # imagen YOLO
│   ├── docker-compose.yml     # compose dev del Visor (yolo + backend)
│   └── firebase-service-account.json  # (NO versionado) clave de servicio
├── maquina_EcoCycle/          ← repo git anidado — app tablet (Kotlin/Compose)
├── EcoCycle-Movil/            ← repo git anidado — app usuario (Kotlin/Compose)
├── CodigoArduino/
│   ├── config.h               # GENERADO (no editar a mano)
│   └── sketch_jun26a_copy_20260629184755/  # firmware ESP32 v5
├── scripts/
│   ├── generate_configs.py    # regenera config.h y EcoCycleConfig.kt desde ecocycle.env
│   ├── nginx-proxy.conf       # reverse proxy de producción (TLS, rate limit, HSTS)
│   └── simulate_flow.py       # simula el flujo completo QR → detección → puntos
├── docs/
│   ├── FLUJO_COMPLETO.md      # arquitectura y flujo end-to-end detallado
│   ├── DEPLOYMENT.md          # guía de despliegue local y VPS
│   └── SECURITY.md            # auditoría y pendientes de seguridad
├── ecocycle.env               # (NO versionado) única fuente de verdad
├── ecocycle.env.example       # plantilla con TODAS las variables documentadas
├── docker-compose.prod.yml    # compose de PRODUCCIÓN (VPS)
├── run.sh                     # orquestador (local y VPS)
├── Makefile                   # orquesta el Visor
└── README.md
```

---

## 5. Flujo end-to-end de la máquina

> Documento detallado: [docs/FLUJO_COMPLETO.md](docs/FLUJO_COMPLETO.md)

| Fase | Quién | Qué hace |
|---|---|---|
| **1. Vinculación** | Tableta + App móvil | La tableta genera `sessionId = machine_001_<timestamp>`, muestra QR y crea el doc `sesiones_reciclaje/{id}` en Firestore. El usuario escanea el QR; la app (ML Kit) escribe su `usuario_id`. La tableta escucha el cambio y habilita "INICIAR". |
| **2. Detección primaria** | Tableta + Visor + YOLO | Al presionar INICIAR, CameraX captura frames 1280×720 (cooldown 1.5 s) y los envía a `POST /detect`. El Visor prueba **Gemini** y si falla hace **fallback a YOLO**. Respuesta: `{botella: true/false}`. |
| **3. Acción física** | Visor + ESP32 | La tableta llama `POST /machine-validate`. El Visor escribe `gate_command` en Firebase RTDB. El ESP32 (poll cada 2 s a `GET /gate-command/{id}`) recibe `openOuter:true`, abre el servo exterior 3 s y confirma con `POST /machine-confirm`. |
| **4. Puntos** | .NET | Al presionar FINALIZAR, la tableta llama `POST /api/SesionReciclaje` (protegido con `X-Api-Key`). El backend acredita **20 pts/botella** y cierra la sesión `estado: "completada"`. |

**Mecanismos de calidad:**
- `STRATEGY_KEEP_ONLY_LATEST` en la cámara de la tableta (no acumula frames).
- Cooldown de **1.5 s** entre detecciones.
- Polling del ESP32 cada **2 s** (balance red/latencia).
- Compresión JPEG ~95% y max body 10 MB (Visor) / 20 MB (YOLO).
- `gate_command` de **consumo único** (el Visor lo borra al leerlo).
- Timeout de 30 s en la tableta si el ESP32 no confirma.

---

## 6. Configuración centralizada

> **`ecocycle.env`** (raíz, NO versionado) es la única fuente de verdad.
> Tras editarlo: `python3 scripts/generate_configs.py` regenera `CodigoArduino/config.h`
> y los `EcoCycleConfig.kt` de ambas apps Android.

```bash
cp ecocycle.env.example ecocycle.env   # y rellenar valores
python3 scripts/generate_configs.py    # regenera configs de dispositivos
```

### Variables (`ecocycle.env.example`)

| Variable | Propósito | Default |
|---|---|---|
| `MACHINE_ID` | Identificador de la máquina | `machine_001` |
| `MACHINE_API_KEY` | Key compartida servidor/tableta/ESP32. `openssl rand -hex 24`. Vacía ⇒ endpoints IoT abiertos (solo dev) | — |
| `VISOR_PORT` | Puerto del Visor NestJS | `3000` |
| `DEBUG_IMAGES` | Guardar imágenes de depuración en disco | `false` |
| `GEMINI_API_KEY` | Detector principal; si falta se usa YOLO | — |
| `GEMINI_MODEL` | Modelo de Gemini | `gemini-2.5-flash` |
| `GEMINI_TIMEOUT_MS` | Timeout de Gemini | `15000` |
| `YOLO_PORT` | Puerto del microservicio YOLO | `8000` |
| `YOLO_CONF_THRESHOLD` | Umbral de confianza de YOLO | `0.35` |
| `YOLO_MODEL_PATH` | Modelo YOLO a cargar | `yolov8n.pt` |
| `FIREBASE_DATABASE_URL` | URL de Firebase Realtime Database | — |
| `FIREBASE_PROJECT_ID` | ID del proyecto Firebase | `ecocycle-e9c04` |
| `FIREBASE_SERVICE_ACCOUNT_KEY` | Archivo de clave de servicio | `firebase-key.json` |
| `NET_API_PORT` | Puerto del backend .NET | `5000` |
| `SMTP_PASSWORD` | Contraseña de aplicación Gmail (envío de correos) | — |
| `WIFI_SSID` / `WIFI_PASSWORD` | Red WiFi del ESP32-CAM | — |
| `IR_SENSOR_PIN` | Pin sensor IR ESP32 | `13` |
| `OUTER_GATE_PIN` | Pin servo compuerta exterior | `12` |
| `INNER_GATE_PIN` | Pin servo compuerta interior | `14` |
| `SERVER_HOST` | IP/dominio del servidor (Visor + .NET) | — |
| `SERVER_DOMAIN` | Dominio público (para URLs HTTPS de las apps) | — |

### Claves de Firebase (NO versionadas)

La **misma** clave de servicio en dos copias:

- `EcoCycle/backend/Back/firebase-key.json`
- `Visor/firebase-service-account.json`

Descargar desde **Firebase Console → Configuración → Cuentas de servicio → Generar clave privada**.

> ⚠️ `config.h` y los `EcoCycleConfig.kt` son **GENERADOS**: no editarlos a mano.
> ⚠️ Nunca versionar `ecocycle.env`, `firebase-key.json` ni `firebase-service-account.json`
> (están en `.gitignore`).

---

## 7. Backend .NET (API REST)

**Proyecto:** `EcoCycle/backend/Back/Back.csproj` — net8.0.
**Persistencia:** Firebase **Firestore** vía SDK `Google.Cloud.Firestore` (patrón repositorio).
**Puerto:** `5000` (`ASPNETCORE_URLS=http://0.0.0.0:5000` en la imagen Docker).

### Paquetes principales

`Google.Cloud.Firestore` 4.3.0, `FirebaseDatabase` 1.0.0, `BCrypt.Net-Next` 4.2.0,
`FluentValidation.AspNetCore` 11.3.1, `JwtBearer` 8.0.8, `SendGrid` 9.29.3,
`Serilog.AspNetCore` 8.0.3, `Swashbuckle.AspNetCore` 6.5.0.

### Endpoints (verificados en los controllers)

| Controller (`api/…`) | Endpoints |
|---|---|
| `Auth` | `POST login`, `POST registro` (fuerza rol `cliente`) |
| `Usuario` | `POST crear-cliente`, `GET perfil`, `GET todos`, `GET {id}`, `PUT {id}`, `DELETE {id}` |
| `SesionReciclaje` | `POST` (**`[MachineApiKey]`**, 20 pts/botella), `GET todas` (`[Authorize]+[AdminOnly]`) |
| `Recompensa` | `GET`, `POST canjear`, `GET mis-canjes`, `PUT canjes/{id}/reclamar`, `GET admin`, `GET canjes/admin`, `POST`, `PUT {id}`, `PUT {id}/estatus` |
| `Productos` | `POST`, `GET`, `GET {id}`, `POST completo`, `PUT completo/{id}`, `GET completo/{id}`, `PUT {id}`, `DELETE {id}` |
| `MateriaPrima` | `GET`, `GET {id}`, `POST`, `PUT {id}`, `DELETE {id}`, `POST {id}/transaccion`, `GET {id}/transacciones` |
| `CompraProducto` | `GET mis-compras`, `GET todas`, `POST`, `PUT {id}/opinion`, `PUT {id}/estado` |
| `compras-proveedores` | `GET`, `POST` |
| `Proveedor` | `GET`, `GET {id}`, `POST`, `PUT {id}`, `DELETE {id}` |
| `Recetas` | `POST`, `GET`, `GET {id}`, `PUT {id}`, `DELETE {id}` |
| `RecetasDetalle` | `POST`, `GET receta/{recetaId}`, `PUT {id}`, `DELETE {id}` |
| `Produccion` | `POST`, `GET`, `GET {id}` |
| `Cotizacion` | `POST`, `GET`, `GET {id}`, `DELETE {id}` |
| `Reportes` | `GET`, `GET {id}`, `POST`, `PUT {id}`, `DELETE {id}` |
| `Comentario` | `POST`, `GET`, `GET publicos`, `GET {id}`, `PUT {id}/responder`, `PUT {id}`, `DELETE {id}` |
| `Notificacion` | `GET`, `PUT leer` |
| `Dashboard` | `GET resumen` |

### Patrón y seguridad

- **Controllers → Services → Repositories**: `Repositories/` (17 concretos) + `Repositories/Interfaces/` (16 interfaces); cada repositorio recibe `FirestoreDb` por constructor y opera una colección (p. ej. `"usuarios"`). DI `AddScoped` en `Program.cs`.
- **Firestore**: `FirestoreDbBuilder` con `ProjectId = ecocycle-e9c04` y `CredentialsPath = firebase-key.json`, registrado como **singleton**.
- **JWT**: valida tokens contra `https://securetoken.google.com/{projectId}`.
- **Login**: `Services/FirebaseAuthService.cs` verifica email+contraseña contra la **Identity Toolkit REST API** de Google. **Nunca** devuelve datos con solo el email.
- **Autorización**: `[Authorize]` + `Auth/AdminOnlyAttribute.cs` (rol admin/administrador en Firestore) + `Auth/MachineApiKeyAttribute.cs` (cabecera `X-Api-Key`, comparación en tiempo fijo).
- **CORS**: `AllowedOrigins = http://localhost:4200, https://ecocyclemx.tech`.
- **Logs**: Serilog (consola + archivo `Logs/log-.txt`, rotación diaria).

---

## 8. Visor NestJS

**Carpeta:** `Visor/src` — NestJS 11. **Puerto:** `process.env.PORT ?? 3000`.

Un solo `AppModule`: `ConfigModule`, `AppController`, y providers `AppService`,
`FirebaseService`, `GeminiService` + guard global `MachineKeyGuard` (exige
`x-api-key` == `MACHINE_API_KEY`; si no está configurada, deja pasar en dev).

### Endpoints

| Endpoint | Método | Función |
|---|---|---|
| `/detect` | POST multipart (`image`, ≤10 MB) | Detecta botella (Gemini → YOLO fallback) |
| `/machine-detect` | POST (`image`, `x-machine-id`) | Como `/detect` pero vinculado a sesión activa; incrementa contador en Firebase |
| `/active-session/:machineId` | GET | Devuelve `{sessionId}` de `sesion_activa` |
| `/machine-validate` | POST (`sessionId`, `machineId`, `esBotella`) | Escribe `validacion1`, activa sesión y crea `gate_command` |
| `/gate-command/:machineId` | GET | Lee y **borra** `gate_command` (consumo único del ESP32) |
| `/machine-confirm` | POST | Escribe `validacion2`; incrementa contador si es botella |
| `/machine-cleanup/:machineId` | POST | Limpia `sesion_activa` y `gate_command` |
| `/session-status/:sessionId` | GET | Estado completo de la sesión |

### Detección: Gemini → YOLO

`detectFromBuffer()`: si `GEMINI_API_KEY` existe, primero intenta **Gemini**; si falla,
hace **fallback a YOLO**.

- **Gemini**: `POST https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent`
  (temp 0), prompt que exige JSON `{botella, objeto, confianza}`; mapea `alta/media/baja → 0.9/0.6/0.3`. Timeout 15 s.
- **YOLO**: HTTP directo a `http://{YOLO_HOST}:{YOLO_PORT}/detect` (default `yolo:8000`
  en compose; timeout 30 s, `application/octet-stream`).

### Firebase (REST, sin SDK)

`firebase.service.ts` usa HTTP directo a **Firebase Realtime Database** con OAuth2
(JWT firmado con la cuenta de servicio, scopes `userinfo.email` + `firebase.database`).
Paths: `sessions/{id}/botellas/count`, `sessions/{id}/validacion1`,
`sessions/{id}/validacion2`, `sessions/{id}/botellas/lastResult`,
`maquinas/{id}/sesion_activa`, `maquinas/{id}/gate_command`.

---

## 9. YOLO (microservicio Python)

**Archivo:** `Visor/yolo_service.py` — servidor HTTP **puro** (`http.server.HTTPServer`,
sin framework). **Puerto:** `YOLO_PORT` (default `8000`).

| Aspecto | Detalle |
|---|---|
| Endpoint | `POST /detect` — bytes de imagen (`application/octet-stream`, máx 20 MB) |
| Respuesta | `{botella: bool, detected_objects: [{name, confidence}]}` |
| Modelo | `yolo11n.pt` (fijado en `Dockerfile.yolo`; fallback `yolov8s.pt`/`yolov8n.pt`) |
| Clase detectada | Solo **COCO 39 `bottle`** (`BOTTLE_CLASSES = {39}`) |
| Umbral | `YOLO_CONF_THRESHOLD` (default `0.35`) |
| Preprocesado | RGB; resize si lado mayor >1280 px; si brillo <50 ⇒ gamma + autocontrast + nitidez, si no brillo 1.1x + nitidez 1.2x |
| Debug | `debug_images/` solo si `DEBUG_IMAGES=true` |
| Dependencias | `ultralytics`, `Pillow`, `numpy` |

---

## 10. Frontend Angular

**Carpeta:** `EcoCycle/frontend/front` — Angular **19.2** (CLI 19.2.17).

### Librerías

`bootstrap` 5.3.8, `bootstrap-icons`, `chart.js` 4.5.1, `sweetalert2` 11.26.25,
`firebase` 11.10.0 + `@angular/fire` 19.2.0, `flatpickr`, `exceljs`, `xlsx`,
`file-saver`, `jspdf` + `jspdf-autotable`, `rxjs`.

### Módulos (`src/app/features/` — 32)

`home`, `login`, `registro`, `nosotros`, `contacto`, `producto`, `catalogo`,
`cliente-panel`, `panel`, `panel-cliente`, `perfil-cliente`, `dashboard-cliente`,
`dashboard-global`, `historial-recompensas`, `compras-cliente`, `notificaciones`,
`soporte`, `configuracion`, `materia-prima`, `usuarios`, `reciclaje` (`sesion-reciclaje`),
`quejas` (`comentarios`), `reportes`, `reportes-nucleo`, `reportes-sistema`,
`compras`, `admin-compras`, `admin-productos`, `recetas`, `produccion`, `proveedores`,
`compras-proveedores`, `cotizaciones`. Carpeta `no_se_estan_usando/` = obsoletos (no enrutados).

### Core y shared

- **`core/guards`**: `authGuard`, `adminGuard`, `clienteGuard`.
- **`core/interceptors`**: `auth.interceptor` (adjunta Bearer token).
- **`core/services`**: auth, usuario, productos, recompensa, sesion-reciclaje, dashboard,
  notificaciones, reporte, comentario, compra, recetas, recetas-detalle, materia-prima,
  produccion, proveedor, cotizaciones, compra-cliente, export, soporte, loader, notification.
- **`shared/`**: `navbar`, `sidebar`, `sidebar-cliente`, `bar`, `app-loader`,
  directiva `flatpickr`.

### Rutas principales

- **Públicas**: `/login`, `/registro`, `/nosotros`, `/contacto`, `/producto`, `/home`, `/error/:code`.
- **`/cliente`** (`clienteGuard`): `mi-perfil/{perfil, mis-compras}`, `catalogo`, `notificaciones`,
  `historial-recompensas`, `dashboard`, `mis-compras`, `soporte`, `configuracion`.
- **`/admin`** (`adminGuard`): `dashboard`, `mi-perfil/*`, `materia-prima`, `usuarios`,
  `reciclaje`, `catalogo`, `historial-recompensas`, `quejas`, `reportes-sistema`,
  `soporte`, `compras`, `comentarios`, `cotizaciones`, `admin-productos`,
  `admin-productos-form`, `admin-productos-form/:id`, `recetas`, `recetas/nueva`,
  `produccion`, `proveedores`, `proveedores/nuevo`, `proveedores/editar/:id`,
  `compras-proveedores`, `compras-proveedores/nueva`, `configuracion`.
- `'' → home`; `** → error/404`.

### Gráficas (Chart.js)

- **Dashboard global/cliente**: gráfica de **área** (botellas recicladas/día) y de
  **pastel** (stock de materia prima). `Chart.register(...registerables)`,
  destruye y recrea el canvas al recargar datos.
- **Dashboard botellas** (obsoleto): gráfica de **barras** (Top 5 recicladores).
- La app se sirve como **build estática** (`dist/front/browser`) por nginx — **el deploy
  debe recompilar el frontend** (ver [CI/CD](#15-cicd-github-actions)).

### Entornos

- `environment.ts` (dev): `apiUrl: http://localhost:5000/api`.
- `environment.prod.ts`: `apiUrl: /api` (relativo, resuelto por nginx).
- Firebase: projectId `ecocycle-e9c04`, authDomain `ecocycle-e9c04.firebaseapp.com`.

---

## 11. Firmware ESP32

**Sketch:** `CodigoArduino/sketch_jun26a_copy_20260629184755/` — **v5 "solo compuerta"**
(sin cámara ni segunda validación). Librerías: `WiFi`, `WiFiClient`, `ArduinoJson`,
`ESP32Servo`, `config.h` (generado).

### Pines (definidos en `config.h`)

| Pin | GPIO | Uso |
|---|---|---|
| `OUTER_GATE_PIN` | 12 | Servo compuerta exterior |
| `INNER_GATE_PIN` | 14 | Servo compuerta interior (definido, no usado en v5) |
| `SENSOR_IR_PIN` | 13 | Sensor IR (previsto, no usado) |
| LED | 4 | Indicador (se enciende al abrir la compuerta) |
| Timer | 2 | PWM para servos |

### Comportamiento

1. Conecta a WiFi (`WIFI_SSID`/`WIFI_PASSWORD` de `config.h`).
2. Cada **2 s** hace `GET http://{VISOR_HOST}:{VISOR_PORT}/gate-command/{MACHINE_ID}`
   con cabecera `X-Api-Key: MACHINE_API_KEY`.
3. Si recibe `openOuter: true` + `sessionId`, abre el servo exterior a **90° durante 3 s**
   (con LED encendido) y regresa a **0°**.
4. Deserializa con `StaticJsonDocument<256>`.

---

## 12. Apps Android

### 12.1 `maquina_EcoCycle` — tableta de la máquina

- **Paquete:** `com.example.maquina_recicladora`. Jetpack Compose, minSdk 26, target/compile 36.
- **Arquitectura:** una sola `MainActivity` con máquina de estados Compose (5 pantallas):
  `Bienvenida` (genera `sessionId`, QR con ZXing, crea doc en Firestore y escucha
  `usuario_id`), `Inicio`, `Validación` (CameraX 1280×720, YUV→JPEG, envía frames al
  Visor con cooldown 1.5 s), `Conteo` (puntos = botellas × 20), `Despedida`.
- **Clases clave:** `MainActivity.kt`, `CameraDetector.kt` (ImageAnalysis
  `STRATEGY_KEEP_ONLY_LATEST`), `ApiClient.kt` (OkHttp 5s/10s), `EcoCycleConfig.kt` (generado).
- **Conexión:** `POST /visor/detect`, `POST /visor/machine-validate`,
  `POST /visor/machine-cleanup/{id}`, `POST /api/sesionreciclaje` — con `X-Api-Key`.
  URLs reales: `https://ecocyclemx.tech/visor` y `https://ecocyclemx.tech/api`.

### 12.2 `EcoCycle-Movil` — app del usuario

- **Paquete:** `com.example.appmovil`. Jetpack Compose, minSdk 26, target 34.
- **Arquitectura:** `LoginActivity` (Firebase Auth) → `MainActivity` con Navigation
  Compose + bottom bar (4 tabs: `dashboard`, `escanear` → `QrActivity`, `recompensas`, `perfil`).
- **QR:** `QrActivity` (XML) con CameraX + **ML Kit BarcodeScanning** (valida QR `machine_`
  y escribe `usuario_id` en Firestore).
- **Red:** Retrofit 2.9 + Gson + OkHttp con `AuthInterceptor` (Bearer token de Firebase).
  Consume: `auth/registro`, `usuario/perfil`, `recompensa`, `recompensa/canjear`,
  `recompensa/mis-canjes`, `notificacion`, `notificacion/leer`, `Dashboard/resumen`.
- **Pantallas:** `AuthScreen`, `DashboardScreen`, `RecompensasScreen`, `MenuScreen`,
  `HistorialScreen`, `AjustesScreen`, `AyudaScreen`, `TerminosScreen`,
  `NotificacionesScreen`, `SesionActivaScreen`.
- Firebase BOM 34.13.0 (auth, analytics, firestore, database), Play Services Auth, Coil.

> ⚠️ **Nota**: `GET /api/usuario/historial` es consumido por el móvil pero **no existe
> como ruta** en `UsuarioController`. Posible endpoint pendiente de implementar.

---

## 13. Correr en local (desarrollo)

### Requisitos

- Docker + Docker Compose
- `ecocycle.env` en la raíz (de `ecocycle.env.example`)
- Claves de Firebase en `EcoCycle/backend/Back/firebase-key.json` y
  `Visor/firebase-service-account.json`
- (Opcional) `GEMINI_API_KEY`; sin ella se usa YOLO

### Comandos

```bash
./run.sh all        # Visor (YOLO + NestJS) + EcoCycle (.NET + Angular) con Docker
./run.sh visor      # solo Visor
./run.sh ecocycle   # solo .NET + Angular
./run.sh test       # prueba la detección con una imagen de ejemplo
./run.sh flow       # simula el flujo completo QR → IoT → puntos
./run.sh logs       # logs de todos los servicios
./run.sh stop       # detiene todo
```

**Alternativa sin Docker (solo Visor):** `make visor-dev`.

### URLs locales

| Servicio | URL |
|---|---|
| Frontend Angular | http://localhost:4200 |
| Backend .NET (Swagger) | http://localhost:5000 |
| Visor NestJS | http://localhost:3000 |
| YOLO | http://localhost:8000 |

---

## 14. Despliegue en el VPS (producción)

**VPS:** DigitalOcean (Ubuntu) · **Dominio:** https://ecocyclemx.tech · **IP:** 104.248.187.43

### Preparación (una sola vez)

```bash
# en el VPS
git clone <repo-raiz> && cd ProyectoFinal
cp ecocycle.env.example ecocycle.env && nano ecocycle.env   # rellenar TODO
# subir las dos claves de Firebase (scp) a sus rutas
```

### Nginx + TLS (una sola vez)

```bash
sudo cp scripts/nginx-proxy.conf /etc/nginx/sites-available/ecocycle
sudo ln -sf /etc/nginx/sites-available/ecocycle /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
sudo certbot --nginx -d ecocyclemx.tech
```

**Ruteo nginx** (único punto de entrada público):

| Ruta pública | Servicio interno |
|---|---|
| `/` | frontend (build estática o `127.0.0.1:4200`) |
| `/api/` | backend .NET (127.0.0.1:5000) |
| `/visor/` | Visor NestJS (127.0.0.1:3000) |

### Desplegar

```bash
./run.sh prod        # build + up de docker-compose.prod.yml (puertos solo en 127.0.0.1)
./run.sh prod-logs   # seguir logs
./run.sh prod-stop   # detener
```

> ⚠️ **Importante:** el frontend se sirve desde la **build estática** de Angular
> (`dist/front/browser`), que está en `.gitignore` y NO se descarga con `git pull`.
> Para publicar cambios del frontend hay que **recompilarlo en el VPS**:
> ```bash
> cd /var/ecocycle/EcoCycle/frontend/front && npm run build -- --configuration production
> sudo systemctl restart nginx
> ```
> El **CI/CD** ya hace esto automáticamente (ver [sección 15](#15-cicd-github-actions)).

Guía completa: [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md).

---

## 15. CI/CD (GitHub Actions)

**Archivo:** `EcoCycle/.github/workflows/deploy.yml` — despliegue automático al VPS.

### Disparadores

- Push a la rama `main` del repo **SuprSalva/EcoCycle**
- Manual (`workflow_dispatch`) desde la pestaña **Actions**

### Qué hace (en el VPS, por SSH)

1. `git fetch origin main` + `git reset --hard origin/main` en `/var/ecocycle/EcoCycle`.
2. `npm install && npm run build -- --configuration production` (recompila el Angular).
3. `docker-compose up -d --build`.
4. `sudo systemctl restart nginx`.

### Secretos requeridos (Settings → Secrets and variables → Actions)

| Secreto | Valor |
|---|---|
| `VPS_HOST` | `104.248.187.43` |
| `VPS_USER` | `root` |
| `VPS_SSH_KEY` | Deploy key privada ed25519 (pública ya en `/root/.ssh/authorized_keys` del VPS) |

> La deploy key se genera con `ssh-keygen -t ed25519`; la pública se agrega al
> `authorized_keys` del VPS y la privada se guarda como secreto. Gratis para repos
> públicos (los minutos de GitHub Actions en runners alojados son gratuitos).
> La build de Angular corre en el **VPS**, no en el runner.

---

## 16. Seguridad

- **Endpoints de máquina** (Visor completo y `POST /api/SesionReciclaje`) exigen
  `X-Api-Key` == `MACHINE_API_KEY` (guard/filtro existentes, comparación en tiempo fijo).
- **`POST /api/Auth/login`** verifica la contraseña vía `FirebaseAuthService`; **nunca**
  devuelve datos con solo el email.
- Endpoints de administración llevan `[Authorize]` + `[AdminOnly]`.
- No se loguean contraseñas ni se devuelve `ex.Message` en respuestas 500.
- `nginx-proxy.conf`: HSTS, X-Frame-Options, Referrer-Policy, X-Content-Type-Options,
  rate limit de 10 r/m en `/api/Auth/login`, `client_max_body_size 15m`.
- **Pendientes manuales** (ver [docs/SECURITY.md](docs/SECURITY.md)): rotar la clave de
  servicio (la actual dio "Invalid JWT Signature", aparentemente revocada), rotar la
  contraseña SMTP de Gmail, y cerrar las reglas de Firebase RTDB (`auth != null`).

---

## 17. Scripts y orquestación

### `run.sh` (raíz)

| Comando | Descripción |
|---|---|
| `all` | Inicia todo en desarrollo (Visor + EcoCycle) |
| `prod` | Despliegue de producción (VPS) |
| `prod-stop` / `prod-logs` | Detener / ver logs de producción |
| `visor` / `ecocycle` | Solo Visor / solo .NET + Angular |
| `stop` | Detener todo |
| `logs` | Logs de todos los servicios |
| `yolo` | YOLO standalone (local) |
| `flow [img] [n]` | QR → IoT → puntos (n botellas) |
| `simulate [args]` | Simulación avanzada |
| `test` | Probar detección con imagen de ejemplo |
| `genconfig` | Regenerar configs desde `ecocycle.env` |
| `setup-fb` | Crear placeholders de Firebase |
| `clean` | Limpiar compilados |
| `help` | Mostrar ayuda |

### `Makefile` (raíz — orquesta solo el Visor)

`all`, `setup-fb`, `visor`, `visor-logs`, `visor-stop`, `visor-restart`, `visor-dev`
(sin Docker), `yolo`, `backend`, `build`, `test-detect`, `clean`, `clean-all`, `help`.

### `scripts/generate_configs.py`

Lee `ecocycle.env` y regenera:

1. `CodigoArduino/config.h` (y lo copia al directorio del sketch).
2. `maquina_EcoCycle/.../EcoCycleConfig.kt` (**con** API key).
3. `EcoCycle-Movil/.../EcoCycleConfig.kt` (**sin** API key).

Genera URLs HTTP (`SERVER_HOST:PORT`) o HTTPS (`https://{SERVER_DOMAIN}/visor` y `/api`).

### `scripts/simulate_flow.py`

Simula el flujo completo (7 pasos): verifica `/active-session`, escribe sesión `linked`,
activa `sesion_activa`, envía N fotos a `/machine-detect`, verifica el contador, marca la
sesión completada y llama `POST /api/sesionreciclaje` (20 pts/botella). Args:
`--image`, `--machine`, `--userId`, `--botellas`, `--skip-firebase`, `--clean`.

---

## 18. Docker Compose

### `docker-compose.prod.yml` (raíz — producción)

| Servicio | Imagen/build | Puertos | Notas |
|---|---|---|---|
| `yolo` | `./Visor` + `Dockerfile.yolo` | — (solo red interna) | `restart: unless-stopped` |
| `visor` | `./Visor` + `Dockerfile.nestjs` | `127.0.0.1:3000:3000` | `YOLO_HOST=yolo`; mount `firebase-service-account.json:ro` |
| `backend` | `./EcoCycle/backend` + `Dockerfile` | `127.0.0.1:5000:5000` | `ASPNETCORE_ENVIRONMENT=Production`; mount `firebase-key.json:ro` |
| `frontend` | `./EcoCycle/frontend` + `Dockerfile` | `127.0.0.1:4200:80` | nginx sirve la SPA |

Todos usan `env_file: ./ecocycle.env`. Solo nginx escucha al exterior (80/443).

### `EcoCycle/docker-compose.yml` (desarrollo — hot-reload)

- `backend`: `dotnet/sdk:8.0`, mount `./backend/Back:/app`, `dotnet watch run` en `0.0.0.0:5000`.
- `frontend`: `node:20`, mount `./frontend/front:/app`, `npm install && npm start -- --host 0.0.0.0` en `4200`.

### `Visor/docker-compose.yml` (desarrollo del Visor)

- `yolo`: puerto `127.0.0.1:8000:8000` (contenedor `argus_yolo`).
- `backend`: puerto `3000:3000` (contenedor `argus_backend`), `YOLO_HOST=yolo`.

---

## 19. Verificación y pruebas

### Post-despliegue (VPS)

```bash
curl -s https://ecocyclemx.tech/                                      # frontend (200)
curl -s https://ecocyclemx.tech/api/Proveedor                         # → 401 (protegido)
curl -s https://ecocyclemx.tech/visor/gate-command/machine_001        # → 401 sin key
curl -s -H "X-Api-Key: $MACHINE_API_KEY" \
     https://ecocyclemx.tech/visor/gate-command/machine_001           # → JSON
```

> Un **401** en los endpoints protegidos es CORRECTO. Un **502** significa que el
> contenedor interno no está arriba: revisar `./run.sh prod-logs`.

### Detección local

```bash
./run.sh test        # POST /detect con imagen de ejemplo
make test-detect     # ídem vía Makefile
```

### Flujo simulado

```bash
./run.sh flow fotoreal.jpg 2        # flujo completo con 2 botellas
./run.sh simulate --image fotoreal.jpg --botellas 3 --clean
```

### Colecciones de API

Carpetas `.bruno/` en `EcoCycle/` con colecciones de pruebas para Bruno.

---

## 20. Solución de problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| El Visor responde 401 a la tableta/ESP32 | API key del dispositivo no coincide | Regenerar configs (`python3 scripts/generate_configs.py`), recompilar app / reflashear ESP32 |
| No llegan correos | Falta `SMTP_PASSWORD` (contraseña de aplicación de Gmail) | Configurar en `ecocycle.env` |
| El login devuelve 401 siempre | `Firebase:WebApiKey` incorrecto o usuario inexistente | Verificar `appsettings.json` y Firebase Auth |
| YOLO no arranca | Primera vez descarga el modelo | Revisar logs con `./run.sh prod-logs` |
| YOLO no detecta botellas | Imagen oscura (sin iluminación) | El preprocesado adaptativo ayuda; mejorar iluminación; ajustar `YOLO_CONF_THRESHOLD` |
| ESP32 no abre la compuerta | Alimentación insuficiente de servos | Alimentación externa 5V; revisar pines `OUTER_GATE_PIN` |
| `ECONNREFUSED` entre servicios | Contenedor no arrancado / red interna | `docker compose up -d`; verificar `YOLO_HOST` |
| El sitio web no refleja cambios del frontend | Build de Angular no regenerada (nginx sirve `dist`) | `npm run build` en el VPS + `systemctl restart nginx` (el CI lo hace solo) |
| `docker compose` da "unknown command" | VPS usa `docker-compose` v1 (sin plugin v2) | Usar `docker-compose` (con guion) en el VPS |
| Build Angular falla por presupuesto | `anyComponentStyle` excede el límite | Subir budgets en `angular.json` (25kb/50kb) |

---

## 21. Documentación adicional

| Documento | Contenido |
|---|---|
| [docs/FLUJO_COMPLETO.md](docs/FLUJO_COMPLETO.md) | Arquitectura y flujo end-to-end detallado de la máquina |
| [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md) | Guía de despliegue local y VPS |
| [docs/SECURITY.md](docs/SECURITY.md) | Auditoría de seguridad y acciones pendientes |
| [EcoCycle_FinalProject.md](EcoCycle_FinalProject.md) | Documento del proyecto final (requerimientos, módulos, diseño) |
| [ANALISIS_COMPARATIVO_IA_ML_DM_BD.md](ANALISIS_COMPARATIVO_IA_ML_DM_BD.md) | Análisis comparativo de tecnologías |

---

*Proyecto Integrador — EcoCycle. "Recicla + Gana." ♻️*
