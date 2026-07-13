---
name: local-dev
description: Levanta y prueba EcoCycle en local (Docker dev con hot-reload). Usar cuando el usuario pida correr el proyecto, probar la detección o simular el flujo de la máquina.
---

# EcoCycle en local

## Levantar

```bash
./run.sh all       # Visor (YOLO+NestJS) + EcoCycle (.NET+Angular), hot-reload
./run.sh visor     # solo la parte de visión
./run.sh ecocycle  # solo .NET + Angular
```

URLs: front :4200, API .NET :5000, Visor :3000, YOLO :8000 (solo localhost).

Si falta `ecocycle.env`: `cp ecocycle.env.example ecocycle.env` y rellenar.
Si faltan las claves Firebase, `./run.sh setup-fb` crea placeholders (la
detección funciona; Firestore/RTDB no).

## Probar

```bash
./run.sh test            # POST /detect con imagen de ejemplo
./run.sh flow            # flujo completo QR → detección → puntos
./run.sh logs            # logs de todos los servicios
```

Nota: si `MACHINE_API_KEY` está definida en `ecocycle.env`, las llamadas
manuales al Visor y a `POST /api/SesionReciclaje` necesitan la cabecera
`X-Api-Key: $MACHINE_API_KEY`.

## Compilar sin Docker

- .NET: `obj/`/`bin/` son de root; copiar `Back/` a un directorio temporal para
  compilar con `dotnet build` (ver CLAUDE.md).
- Visor: `cd Visor && npm run build`.
- YOLO: `python3 -m py_compile Visor/yolo_service.py`.
