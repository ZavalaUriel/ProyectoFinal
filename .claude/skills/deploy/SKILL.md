---
name: deploy
description: Despliega EcoCycle en el VPS de producción (docker-compose.prod.yml + nginx). Usar cuando el usuario pida desplegar, actualizar producción o verificar el despliegue.
---

# Desplegar EcoCycle en producción

## Pre-requisitos (verificar antes de desplegar)

1. `ecocycle.env` existe en la raíz y tiene `MACHINE_API_KEY`, `SMTP_PASSWORD`,
   `SERVER_DOMAIN` no vacíos. Si falta, detenerse y avisar al usuario.
2. Existen `EcoCycle/backend/Back/firebase-key.json` y
   `Visor/firebase-service-account.json` con claves reales (el placeholder tiene
   `"private_key":"placeholder"`).

## Desplegar

```bash
./run.sh prod          # build + up (idempotente, reconstruye solo lo cambiado)
./run.sh prod-logs     # seguir logs si algo falla
```

En el VPS, nginx debe estar configurado una sola vez:

```bash
sudo cp scripts/nginx-proxy.conf /etc/nginx/sites-available/ecocycle
sudo ln -sf /etc/nginx/sites-available/ecocycle /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
```

## Verificar

```bash
curl -sf https://ecocyclemx.tech/ >/dev/null && echo FRONT_OK
curl -s https://ecocyclemx.tech/api/Proveedor | head -c 200        # espera 401
curl -s https://ecocyclemx.tech/visor/gate-command/machine_001     # espera 401 sin key
```

Un 401 en los dos últimos es CORRECTO (endpoints protegidos). Un 502 significa
que el contenedor interno no está arriba: revisar `./run.sh prod-logs`.

## Si cambió ecocycle.env

Regenerar configs de dispositivos y avisar que hay que recompilar apps/ESP32:

```bash
python3 scripts/generate_configs.py
```
