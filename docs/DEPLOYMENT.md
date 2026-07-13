# Despliegue de EcoCycle

Hay dos modos: **local (desarrollo)** con hot-reload y **producción (VPS)** con
imágenes optimizadas y puertos cerrados al exterior.

## Requisitos comunes

1. `ecocycle.env` en la raíz (copiar de `ecocycle.env.example` y rellenar):
   - `MACHINE_API_KEY`: generar con `openssl rand -hex 24`
   - `SMTP_PASSWORD`: contraseña de aplicación de Gmail
   - `GEMINI_API_KEY` (opcional; sin ella se usa YOLO)
   - `SERVER_HOST` / `SERVER_DOMAIN` (solo VPS)
2. Clave de servicio de Firebase (Console → Cuentas de servicio → Generar clave):
   - `EcoCycle/backend/Back/firebase-key.json`
   - `Visor/firebase-service-account.json` (misma clave)
3. Docker + Docker Compose.

> Si cambias `ecocycle.env`, regenera las configs de dispositivos:
> `python3 scripts/generate_configs.py` (y recompila apps Android / re-flashea ESP32).

## Local (desarrollo)

```bash
./run.sh all          # todo con Docker (hot-reload en .NET y Angular)
./run.sh visor        # solo Visor (YOLO + NestJS)
./run.sh ecocycle     # solo .NET + Angular
./run.sh logs         # ver logs
./run.sh stop         # detener
```

Sin Docker (Visor): `make visor-dev` (requiere `pip install ultralytics Pillow`
y `npm install` en `Visor/`).

Prueba rápida:

```bash
./run.sh test   # POST /detect con imagen de ejemplo
./run.sh flow   # flujo completo QR → detección → puntos
```

## Producción (VPS)

### 1. Preparar el servidor (una sola vez)

```bash
# en el VPS
git clone <repo> && cd ProyectoFinal
cp ecocycle.env.example ecocycle.env && nano ecocycle.env   # rellenar TODO
# subir las dos claves de Firebase (scp) a las rutas indicadas arriba
```

### 2. Nginx + TLS

```bash
sudo cp scripts/nginx-proxy.conf /etc/nginx/sites-available/ecocycle
sudo ln -s /etc/nginx/sites-available/ecocycle /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
sudo certbot --nginx -d ecocyclemx.tech   # certificado TLS
```

Nginx es el único punto de entrada público:

| Ruta pública | Servicio interno |
|---|---|
| `/` | frontend (127.0.0.1:4200) |
| `/api/` | backend .NET (127.0.0.1:5000) |
| `/visor/` | Visor NestJS (127.0.0.1:3000) |

### 3. Desplegar

```bash
./run.sh prod        # build + up de docker-compose.prod.yml
./run.sh prod-logs   # seguir logs
./run.sh prod-stop   # detener
```

### 4. Actualizar (redeploy)

```bash
git pull --recurse-submodules
./run.sh prod        # reconstruye solo lo que cambió
```

## Verificación post-despliegue

```bash
curl -s https://ecocyclemx.tech/api/Proveedor            # → 401 (protegido)
curl -s https://ecocyclemx.tech/visor/gate-command/machine_001   # → 401 sin X-Api-Key
curl -s -H "X-Api-Key: $MACHINE_API_KEY" https://ecocyclemx.tech/visor/gate-command/machine_001  # → JSON
```

## Problemas comunes

- **El Visor responde 401 a la tablet/ESP32**: la API key del dispositivo no
  coincide. Regenerar configs (`python3 scripts/generate_configs.py`),
  recompilar app / re-flashear ESP32.
- **No llegan correos**: falta `SMTP_PASSWORD` en `ecocycle.env` (contraseña de
  aplicación de Gmail, no la contraseña normal).
- **El login devuelve 401 siempre**: verificar `Firebase:WebApiKey` en
  `appsettings.json` y que el usuario exista en Firebase Authentication.
- **YOLO no arranca**: la primera vez descarga el modelo; ver logs con
  `./run.sh prod-logs`.
