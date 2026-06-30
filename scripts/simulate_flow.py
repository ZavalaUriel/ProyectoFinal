#!/usr/bin/env python3
"""
Simulación del flujo completo EcoCycle
Uso: python3 scripts/simulate_flow.py [--image ruta] [--botellas N]

Simula:
  1. Tablet genera QR (sessionId)
  2. Móvil escanea QR y escribe en Firebase
  3. Tablet detecta link y activa sesión
  4. ESP32 envía N fotos al Visor (YOLO detecta)
  5. Visor incrementa contador por cada botella detectada
  6. Tablet finaliza y llama .NET API
  7. Limpieza de datos de prueba
"""

import argparse
import io
import json
import os
import re
import sys
import time
import uuid

try:
    import requests
except ImportError:
    print("❌ Falta requests: pip install requests")
    sys.exit(1)

# ─── Cargar .env centralizado ──────────────────────────────────────────────
ENV_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "ecocycle.env")
if os.path.exists(ENV_PATH):
    with open(ENV_PATH) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)=(.*)$", line)
            if m and m.group(1) not in os.environ:
                os.environ[m.group(1)] = m.group(2)

# ─── Configuración ──────────────────────────────────────────────────────────
VISOR_HOST = os.environ.get("VISOR_HOST", "localhost")
VISOR_PORT = os.environ.get("VISOR_PORT", "3000")
YOLO_PORT = os.environ.get("YOLO_PORT", "8000")
FIREBASE_URL = os.environ.get(
    "FIREBASE_URL",
    "https://ecocycle-e9c04-default-rtdb.firebaseio.com",
)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
DEFAULT_IMAGE = os.path.join(PROJECT_DIR, "Visor", "descarga.jpeg")

VISOR_API = f"http://{VISOR_HOST}:{VISOR_PORT}"
NET_API = os.environ.get("NET_API_URL", "http://localhost:5000")

PTS_POR_BOTELLA = 20


# ─── Utilerías ──────────────────────────────────────────────────────────────

def p(msg: str):
    print(f"  {msg}")


def ok(msg: str):
    print(f"  ✅ {msg}")


def fail(msg: str):
    print(f"  ❌ {msg}")


def warn(msg: str):
    print(f"  ⚠️  {msg}")


def step(n: int, total: int, name: str):
    print(f"\n─── Paso {n}/{total}: {name} ───")


def firebase_get(path: str):
    url = f"{FIREBASE_URL}/{path}.json"
    r = requests.get(url, timeout=10)
    return r.json() if r.status_code == 200 else None


def firebase_put(path: str, data):
    url = f"{FIREBASE_URL}/{path}.json"
    r = requests.put(url, json=data, timeout=10)
    return r.status_code == 200


def firebase_delete(path: str):
    url = f"{FIREBASE_URL}/{path}.json"
    r = requests.delete(url, timeout=10)
    return r.status_code == 200


# ─── Simulación ─────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="Simular flujo completo EcoCycle")
    parser.add_argument("--image", default=DEFAULT_IMAGE, help="Ruta de la imagen de prueba")
    parser.add_argument("--machine", default="machine_001", help="ID de la máquina")
    parser.add_argument("--userId", default="simulacion_user_001", help="ID de usuario simulado")
    parser.add_argument("--botellas", type=int, default=1, help="Cantidad de botellas a simular")
    parser.add_argument("--skip-firebase", action="store_true", help="Omitir pasos de Firebase")
    parser.add_argument("--clean", action="store_true", help="Limpiar datos de prueba al final")
    args = parser.parse_args()

    if not os.path.exists(args.image):
        print(f"❌ Imagen no encontrada: {args.image}")
        sys.exit(1)

    if args.botellas < 1:
        print("❌ --botellas debe ser al menos 1")
        sys.exit(1)

    total_steps = 7 if not args.skip_firebase else 4
    session_id = f"{args.machine}_{int(time.time() * 1000)}"

    # Pre-cargar imagen (se reusa para cada botella)
    is_png = args.image.lower().endswith(".png")
    content_type = "image/png" if is_png else "image/jpeg"
    filename = "capture.png" if is_png else "capture.jpg"

    with open(args.image, "rb") as f:
        img_bytes = f.read()

    if is_png:
        from PIL import Image as PilImage
        buf = io.BytesIO()
        PilImage.open(io.BytesIO(img_bytes)).convert("RGB").save(buf, "JPEG")
        img_bytes = buf.getvalue()
        filename = "capture.jpg"
        content_type = "image/jpeg"

    print("╔══════════════════════════════════════════════╗")
    print("║   Simulación EcoCycle - Flujo Completo       ║")
    print("╠══════════════════════════════════════════════╣")
    print(f"║  Sesión:    {session_id}")
    print(f"║  Máquina:   {args.machine}")
    print(f"║  Usuario:   {args.userId}")
    print(f"║  Imagen:    {args.image}")
    print(f"║  Botellas:  {args.botellas}")
    print(f"║  Visor API: {VISOR_API}")
    print(f"║  .NET API:  {NET_API}")
    print("╚══════════════════════════════════════════════╝")

    # ── Paso 1: Verificar que el Visor responda ──
    step(1, total_steps, "Verificar Visor")
    try:
        r = requests.get(f"{VISOR_API}/active-session/{args.machine}", timeout=5)
        ok(f"Visor responde en {VISOR_API}")
    except requests.ConnectionError:
        fail(f"No se puede conectar a {VISOR_API}")
        print("\n   ¿Corriste './run.sh visor' primero?")
        sys.exit(1)

    # ── Paso 2: Simular móvil escanea QR y escribe en Firebase ──
    if not args.skip_firebase:
        step(2, total_steps, "Móvil escanea QR → Firebase")
        data = {
            "linked": True,
            "userId": args.userId,
            "userEmail": "simulacion@test.com",
            "linkedAt": int(time.time() * 1000),
            "status": "vinculado",
        }
        path = f"sessions/{session_id}"
        if firebase_put(path, data):
            ok(f"sessions/{session_id} creado con linked=true, userId={args.userId}")
        else:
            warn("No se pudo escribir en Firebase (público?). Continuando...")

        result = firebase_get(path)
        if result and result.get("linked"):
            ok(f"Firebase confirma: linked={result['linked']}, userId={result.get('userId')}")
        else:
            warn("Firebase no confirmó (revisa las reglas de seguridad)")
    else:
        step(2, total_steps, "Móvil escanea QR → Firebase (skipped)")
        p("--skip-firebase activo, se omite")

    # ── Paso 3: Simular tablet escribe sesión activa ──
    if not args.skip_firebase:
        step(3, total_steps, "Tablet activa sesión en máquina")
        machine_path = f"maquinas/{args.machine}/sesion_activa"
        if firebase_put(machine_path, session_id):
            ok(f"maquinas/{args.machine}/sesion_activa = {session_id}")
        else:
            warn("No se pudo escribir en Firebase")

        result = firebase_get(machine_path)
        if result == session_id:
            ok(f"Firebase confirma sesión activa: {result}")
        else:
            warn("Firebase no confirma")
    else:
        step(3, total_steps, "Tablet activa sesión (skipped)")
        p("--skip-firebase activo, se omite")

    # ── Paso 4: Simular ESP32 envía N fotos al Visor ──
    paso_actual = 4 if args.skip_firebase else 4
    label = "ESP32 envía foto" if args.skip_firebase else "ESP32 envía fotos al Visor"
    step(paso_actual, total_steps, label)

    botellas_detectadas = 0
    for i in range(args.botellas):
        p(f"\n  📸 Foto {i + 1}/{args.botellas}...")

        files = {"image": (filename, io.BytesIO(img_bytes), content_type)}
        headers = {"X-Machine-Id": args.machine}
        r = requests.post(
            f"{VISOR_API}/machine-detect",
            files=files,
            headers=headers,
            timeout=30,
        )

        if r.status_code in (200, 201):
            result = r.json()
            botella = result.get("botella", False)
            objs = result.get("detected_objects", [])
            sid = result.get("sessionId", "N/A")

            if botella:
                botellas_detectadas += 1

            if objs:
                for obj in objs[:3]:
                    p(f"    → {obj['name']} ({obj['confidence']:.2%})")

            if botella:
                ok(f"🎯 Botella #{i + 1} DETECTADA → compuerta interior")
            else:
                p(f"    → NO es botella → compuerta exterior")
        else:
            fail(f"Visor respondió {r.status_code}: {r.text[:100]}")

        time.sleep(0.3)

    # ── Paso 5: Verificar conteo en Firebase ──
    if not args.skip_firebase:
        step(5, total_steps, "Verificar conteo en Firebase")
        path = f"sessions/{session_id}/botellas"
        result = firebase_get(path)
        if result:
            count = result.get("count", 0)
            ok(f"Botellas contadas en Firebase: {count}")
            p(f"  Puntos estimados: {count * PTS_POR_BOTELLA}")
        else:
            warn("No hay datos de botellas en Firebase aún")
            p("  (Tal vez YOLO no detectó botella en la imagen de prueba)")
    else:
        step(5, total_steps, "Verificar Firebase (skipped)")

    # ── Paso 6: Tablet finaliza sesión ──
    if not args.skip_firebase:
        step(6, total_steps, "Tablet finaliza sesión")
        result = firebase_get(f"sessions/{session_id}/botellas/count")
        botellas_total = result if isinstance(result, int) else 0
        puntos = botellas_total * PTS_POR_BOTELLA

        p(f"  Botellas: {botellas_total}")
        p(f"  Puntos:   {puntos}")

        firebase_put(f"sessions/{session_id}/status", "completado")
        firebase_put(f"maquinas/{args.machine}/sesion_activa", None)

        ok("Sesión marcada como completada en Realtime DB")

        # Llamar al backend .NET para persistir en Firestore
        try:
            payload = {
                "usuarioId": args.userId,
                "maquinaId": args.machine,
                "botellas": botellas_total
            }
            r2 = requests.post(
                f"{NET_API}/api/sesionreciclaje",
                json=payload,
                timeout=5,
                headers={"Content-Type": "application/json"}
            )
            if r2.status_code in (200, 201):
                data = r2.json()
                ok(f"Sesión registrada en Firestore (.NET API): {json.dumps(data)}")
            else:
                warn(f".NET API respondió {r2.status_code}: {r2.text[:100]}")
        except requests.ConnectionError:
            warn(f".NET API no disponible en {NET_API}/api/sesionreciclaje")
    else:
        step(6, total_steps, "Tablet finaliza sesión (skipped)")

    # ── Paso 7: Limpiar ──
    step(7, total_steps, "Limpieza de datos de prueba")
    if args.clean and not args.skip_firebase:
        firebase_delete(f"sessions/{session_id}")
        firebase_delete(f"maquinas/{args.machine}/sesion_activa")
        ok("Datos de prueba eliminados de Realtime DB")
    else:
        p("Datos conservados. Para limpiar: --clean")
        p(f"  O manual: firebase sessions/{session_id}")

    # ── Resumen ──
    puntos_totales = botellas_detectadas * PTS_POR_BOTELLA
    print("\n" + "═" * 50)
    print("RESUMEN")
    print("═" * 50)
    print(f"  Sesión:    {session_id}")
    print(f"  Máquina:   {args.machine}")
    print(f"  Usuario:   {args.userId}")
    print(f"  Imagen:    {args.image}")
    print(f"  Intentos:  {args.botellas}")
    print(f"  Detectadas:{botellas_detectadas}")
    print(f"  Puntos:    {puntos_totales}")
    if not args.skip_firebase:
        count = firebase_get(f"sessions/{session_id}/botellas/count") or 0
        print(f"  Firebase:  {count} botellas")
    print("═" * 50)
    print()
    print("📱 QR para móvil real:")
    print(f"   {session_id}")
    print()
    print("🔍 Ver imagen enviada:")
    print(f"   POST http://localhost:3000/detect (form-data image)")
    print()


if __name__ == "__main__":
    main()
