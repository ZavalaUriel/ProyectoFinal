# EcoCycle — guía para Claude Code

Sistema de reciclaje: máquina IoT + visión artificial + puntos canjeables.
Detalle de componentes en [README.md](README.md); flujo end-to-end en
[docs/FLUJO_COMPLETO.md](docs/FLUJO_COMPLETO.md).

## Estructura (multi-repo)

La raíz es un repo git; `EcoCycle`, `EcoCycle-Movil`, `Visor` y
`maquina_EcoCycle` son repos git anidados (tratados como submódulos sin
`.gitmodules`). Commits van en el repo correspondiente a cada carpeta.

- `EcoCycle/backend/Back` — API .NET 8 (Firestore). Controllers → Repositories.
- `EcoCycle/frontend/front` — Angular (NO tocar sin pedirlo el usuario).
- `Visor/src` — NestJS: detección (Gemini → fallback YOLO) y sesiones de máquina.
- `Visor/yolo_service.py` — microservicio Python HTTP puro (sin framework).
- `maquina_EcoCycle`, `EcoCycle-Movil` — apps Android Kotlin.
- `CodigoArduino` — sketch ESP32 (compuerta).

## Configuración

- `ecocycle.env` (raíz, NO versionado) es la única fuente de verdad.
  Tras cambiarlo: `python3 scripts/generate_configs.py` regenera
  `CodigoArduino/config.h` y los `EcoCycleConfig.kt` de las apps.
- Los `EcoCycleConfig.kt` y `config.h` son GENERADOS: no editarlos a mano.
- Secretos: `SMTP_PASSWORD` y `MACHINE_API_KEY` en `ecocycle.env`;
  claves Firebase en `EcoCycle/backend/Back/firebase-key.json` y
  `Visor/firebase-service-account.json` (no versionadas).

## Seguridad (no regresionar)

- `POST /api/Auth/login` verifica la contraseña vía `FirebaseAuthService`;
  nunca devolver datos con solo el email.
- Endpoints de administración llevan `[Authorize]` + `[AdminOnly]`.
- Endpoints de máquina (Visor completo y `POST /api/SesionReciclaje`) exigen
  `X-Api-Key` == `MACHINE_API_KEY` (guard/filtro ya existentes).
- No loguear contraseñas ni devolver `ex.Message` en respuestas 500.
- Ver [docs/SECURITY.md](docs/SECURITY.md) antes de tocar auth.

## Builds y verificación

- .NET: `obj/`/`bin/` del proyecto son de root (Docker); compilar copiando a un
  directorio temporal o dentro de Docker. `dotnet build Back.csproj`.
- Visor: `cd Visor && npm run build`; YOLO: `python3 -m py_compile yolo_service.py`.
- Local todo: `./run.sh all`; producción VPS: `./run.sh prod`.
- Las apps Android se compilan solo con Android Studio (no hay CI).
