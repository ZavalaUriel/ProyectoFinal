# EcoCycle

Sistema de reciclaje inteligente: una máquina IoT valida botellas PET con visión
artificial y acredita puntos canjeables a los usuarios.

## Componentes

| Componente | Carpeta | Tecnología | Rol |
|---|---|---|---|
| Backend API | `EcoCycle/backend` | .NET 8 + Firestore | Usuarios, puntos, recompensas, reportes |
| Frontend web | `EcoCycle/frontend` | Angular + Firebase Auth | Panel de clientes y administradores |
| Visor | `Visor` | NestJS + Gemini/YOLO | Detección de botellas y orquestación de la máquina |
| YOLO service | `Visor/yolo_service.py` | Python + ultralytics | Detección de respaldo cuando Gemini no está disponible |
| App máquina | `maquina_EcoCycle` | Android (Kotlin) | Tablet de la máquina recicladora |
| App móvil | `EcoCycle-Movil` | Android (Kotlin) | App del usuario final |
| ESP32 | `CodigoArduino` | Arduino | Compuerta física de la máquina |

## Configuración

Toda la configuración vive en **`ecocycle.env`** (no versionado; contiene secretos).

```bash
cp ecocycle.env.example ecocycle.env   # y rellenar valores
python3 scripts/generate_configs.py    # regenera configs de ESP32 y apps Android
```

Claves de Firebase (misma clave de servicio, dos copias):

- `EcoCycle/backend/Back/firebase-key.json`
- `Visor/firebase-service-account.json`

Descargar desde Firebase Console → Configuración → Cuentas de servicio → Generar clave privada.

## Correr en local (desarrollo)

```bash
./run.sh all        # Visor (YOLO + NestJS) + EcoCycle (.NET + Angular) con Docker
./run.sh test       # prueba la detección con una imagen de ejemplo
./run.sh flow       # simula el flujo completo QR → IoT → puntos
./run.sh stop       # detiene todo
```

URLs locales: front `http://localhost:4200`, API `http://localhost:5000`,
Visor `http://localhost:3000`, YOLO `http://localhost:8000` (solo localhost).

## Desplegar en el VPS (producción)

```bash
./run.sh prod       # imágenes de producción, puertos solo en 127.0.0.1
```

El único punto de entrada público es nginx con TLS (`scripts/nginx-proxy.conf`).
Guía completa: [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md).

## Seguridad

Los endpoints de la máquina IoT exigen la cabecera `X-Api-Key` (valor de
`MACHINE_API_KEY` en `ecocycle.env`); las apps y el ESP32 la reciben vía
`generate_configs.py`. El estado de seguridad y las acciones pendientes están en
[docs/SECURITY.md](docs/SECURITY.md).

## Documentación

- [docs/FLUJO_COMPLETO.md](docs/FLUJO_COMPLETO.md) — flujo end-to-end de la máquina
- [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md) — despliegue local y VPS
- [docs/SECURITY.md](docs/SECURITY.md) — hallazgos de seguridad y pendientes
