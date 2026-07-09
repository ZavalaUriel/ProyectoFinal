#!/usr/bin/env python3
"""
Genera archivos de configuración a partir de ecocycle.env
para ESP32-CAM y Apps Android.

Uso:
  python3 scripts/generate_configs.py

Esto lee ecocycle.env y genera:
  - CodigoArduino/config.h        (para ESP32-CAM)
  - maquina_EcoCycle/app/src/main/java/.../Config.kt
  - EcoCycle-Movil/app/src/main/java/.../Config.kt
"""

import glob
import os
import re

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENV_PATH = os.path.join(PROJECT_DIR, "ecocycle.env")

# ─── Parsing del .env ────────────────────────────────────────────────────────

def parse_env(path: str) -> dict[str, str]:
    env = {}
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)=(.*)$", line)
            if m:
                key, val = m.group(1), m.group(2)
                val = val.strip("\"'")
                env[key] = val
    return env


# ─── Generar config.h para ESP32-CAM ─────────────────────────────────────────

def generate_esp32_config(env: dict[str, str]) -> str:
    lines = [
        "// Auto-generado desde ecocycle.env. NO EDITAR MANUALMENTE.",
        "// Regenerar con: python3 scripts/generate_configs.py",
        "",
        "#ifndef ECOCYCLE_CONFIG_H",
        "#define ECOCYCLE_CONFIG_H",
        "",
        "// WiFi",
        f'#define WIFI_SSID "{env.get("WIFI_SSID", "Totalplay-C8B1")}"',
        f'#define WIFI_PASSWORD "{env.get("WIFI_PASSWORD", "")}"',
        "",
        "// Visor (NestJS)",
        f'#define VISOR_HOST "{env.get("SERVER_HOST", "192.168.100.19")}"',
        f'#define VISOR_PORT {env.get("VISOR_PORT", "3000")}',
        "",
        "// Identificador de máquina",
        f'#define MACHINE_ID "{env.get("MACHINE_ID", "machine_001")}"',
        "",
        "// Pines",
        f'#define SENSOR_IR_PIN {env.get("IR_SENSOR_PIN", "13")}',
        f'#define OUTER_GATE_PIN {env.get("OUTER_GATE_PIN", "12")}',
        f'#define INNER_GATE_PIN {env.get("INNER_GATE_PIN", "14")}',
        "",
        "#endif",
        "",
    ]
    return "\n".join(lines)


# ─── Generar Config.kt para Android ──────────────────────────────────────────

ANDROID_PACKAGES = {
    "maquina_EcoCycle": "com.example.maquina_recicladora",
    "EcoCycle-Movil": "com.example.appmovil",
}


def generate_android_config(env: dict[str, str]) -> str:
    server_host = env.get("SERVER_HOST", "192.168.100.19")
    server_domain = env.get("SERVER_DOMAIN", "")
    visor_port = env.get("VISOR_PORT", "3000")
    net_port = env.get("NET_API_PORT", "5000")
    machine_id = env.get("MACHINE_ID", "machine_001")

    https_visor = f'"https://{server_domain}/visor"' if server_domain else f'"http://${{SERVER_HOST}}:${{VISOR_PORT}}"'
    https_net = f'"https://{server_domain}/api"' if server_domain else f'"http://${{SERVER_HOST}}:${{NET_API_PORT}}/api"'

    lines = [
        "// Auto-generado desde ecocycle.env. NO EDITAR MANUALMENTE.",
        "// Regenerar con: python3 scripts/generate_configs.py",
        "",
        f"package {ANDROID_PACKAGES['maquina_EcoCycle']}",
        "",
        "object EcoCycleConfig {",
        f'    const val MACHINE_ID = "{machine_id}"',
        f'    const val SERVER_HOST = "{server_host}"',
        f'    const val SERVER_DOMAIN = "{server_domain}"' if server_domain else '',
        f'    const val VISOR_PORT = {visor_port}',
        f'    const val NET_API_PORT = {net_port}',
        f'    const val VISOR_URL_HTTPS = {https_visor}',
        f'    const val NET_API_URL_HTTPS = {https_net}',
        f'    val VISOR_URL: String get() = "https://${{SERVER_DOMAIN}}/visor"',
        f'    val NET_API_URL: String get() = "https://${{SERVER_DOMAIN}}/api"',
        "}",
        "",
    ]
    return "\n".join(lines)


# ─── Main ────────────────────────────────────────────────────────────────────

def main():
    if not os.path.exists(ENV_PATH):
        print(f"❌ No se encuentra {ENV_PATH}")
        print("   Crear desde ecocycle.env.example o editar el existente.")
        return

    env = parse_env(ENV_PATH)
    ok = []
    fail = []

    # ESP32
    esp32_dst = os.path.join(PROJECT_DIR, "CodigoArduino", "config.h")
    esp32_content = generate_esp32_config(env)
    try:
        os.makedirs(os.path.dirname(esp32_dst), exist_ok=True)
        with open(esp32_dst, "w") as f:
            f.write(esp32_content)
        ok.append(f"✅ ESP32: {esp32_dst}")
        # También copiar al directorio del sketch
        sketch_dirs = glob.glob(os.path.join(PROJECT_DIR, "CodigoArduino", "sketch_*"))
        for sketch_dir in sketch_dirs:
            sketch_config = os.path.join(sketch_dir, "config.h")
            with open(sketch_config, "w") as f:
                f.write(esp32_content)
            ok.append(f"✅ ESP32 sketch: {sketch_config}")
    except OSError as e:
        fail.append(f"❌ ESP32: {e}")

    # Android (maquina_EcoCycle - tablet)
    tablet_pkg = ANDROID_PACKAGES["maquina_EcoCycle"]
    tablet_path = os.path.join(
        PROJECT_DIR, "maquina_EcoCycle",
        "app", "src", "main", "java",
        *tablet_pkg.split("."),
        "EcoCycleConfig.kt",
    )
    android_content = generate_android_config(env)
    try:
        os.makedirs(os.path.dirname(tablet_path), exist_ok=True)
        with open(tablet_path, "w") as f:
            f.write(android_content)
        ok.append(f"✅ Tablet: {tablet_path}")
    except OSError as e:
        fail.append(f"❌ Tablet: {e}")

    # Android (EcoCycle-Movil - mobile)
    mobile_pkg = ANDROID_PACKAGES["EcoCycle-Movil"]
    mobile_path = os.path.join(
        PROJECT_DIR, "EcoCycle-Movil",
        "app", "src", "main", "java",
        *mobile_pkg.split("."),
        "EcoCycleConfig.kt",
    )
    # Mobile needs different package in the generated file
    mobile_content = generate_android_config(env).replace(
        f"package {ANDROID_PACKAGES['maquina_EcoCycle']}",
        f"package {ANDROID_PACKAGES['EcoCycle-Movil']}",
    )
    try:
        os.makedirs(os.path.dirname(mobile_path), exist_ok=True)
        with open(mobile_path, "w") as f:
            f.write(mobile_content)
        ok.append(f"✅ Móvil: {mobile_path}")
    except OSError as e:
        fail.append(f"❌ Móvil: {e}")

    print("\n" + "=" * 50)
    print("Configuración generada desde ecocycle.env")
    print("=" * 50)
    for msg in ok:
        print(f"  {msg}")
    for msg in fail:
        print(f"  {msg}")
    print()


if __name__ == "__main__":
    main()
