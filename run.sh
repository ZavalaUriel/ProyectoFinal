#!/usr/bin/env bash
# EcoCycle - Script de orquestación
# Uso: ./run.sh <comando>
# Comandos: all, visor, ecocycle, visor-stop, ecocycle-stop, yolo, test, help

set -e

SHELL_DIR="$(cd "$(dirname "$0")" && pwd)"
VISOR_DIR="$SHELL_DIR/Visor"
ECO_DIR="$SHELL_DIR/EcoCycle"

# Cargar configuración centralizada
ENV_FILE="$SHELL_DIR/ecocycle.env"
if [ -f "$ENV_FILE" ]; then
  set -a
  source "$ENV_FILE"
  set +a
fi

setup_fb_visor() {
  if [ ! -f "$VISOR_DIR/firebase-service-account.json" ]; then
    cat > "$VISOR_DIR/firebase-service-account.json" <<'EOF'
{"type":"service_account","project_id":"ecocycle-e9c04","private_key_id":"placeholder","private_key":"placeholder","client_email":"placeholder@placeholder.com","client_id":"0","auth_uri":"https://accounts.google.com/o/oauth2/auth","token_uri":"https://oauth2.googleapis.com/token"}
EOF
    echo "⚠️  Creado placeholder firebase-service-account.json (Visor)"
    echo "   Reemplázalo con tu clave real desde Firebase Console"
  fi
}

setup_fb_ecocycle() {
  if [ ! -f "$ECO_DIR/backend/Back/firebase-key.json" ]; then
    echo ""
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║  ❌ Falta la clave de Firebase para el backend .NET         ║"
    echo "║                                                              ║"
    echo "║  Descárgala desde:                                           ║"
    echo "║  https://console.firebase.google.com/project/ecocycle-e9c04/║"
    echo "║  settings/serviceaccounts/adminsdk                         ║"
    echo "║                                                              ║"
    echo "║  → Generar nueva clave privada (JSON)                       ║"
    echo "║  → Guardarla como:                                           ║"
    echo "║    $ECO_DIR/backend/Back/firebase-key.json                   ║"
    echo "║                                                              ║"
    echo "║  También para el Visor:                                     ║"
    echo "║    $VISOR_DIR/firebase-service-account.json                  ║"
    echo "║                                                              ║"
    echo "║  Es el MISMO archivo, puedes copiarlo a ambos.              ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo ""
    echo "Presiona Enter cuando hayas descargado la clave..."
    read -r
    if [ -f "$ECO_DIR/backend/Back/firebase-key.json" ]; then
      echo "✅ Clave encontrada"
    else
      echo "⚠️  No se encontró. El backend .NET no funcionará sin la clave."
    fi
  fi
}

setup_fb() {
  setup_fb_visor
  setup_fb_ecocycle
}

# ─── Visor (YOLO + NestJS) ──────────────────────────────────────────────────

cmd_visor() {
  setup_fb_visor
  echo "🚀 Iniciando Visor (YOLO + NestJS)..."
  cd "$VISOR_DIR"
  docker compose up --build -d
  sleep 3
  echo "✅ Visor listo"
  echo "   YOLO:  http://localhost:8000"
  echo "   Visor: http://localhost:3000"
}

cmd_visor_stop() {
  cd "$VISOR_DIR" && docker compose down 2>/dev/null || true
}

# ─── EcoCycle (.NET Backend + Angular Frontend) ──────────────────────────────

cmd_ecocycle() {
  setup_fb_ecocycle
  echo "🚀 Iniciando EcoCycle (.NET + Angular)..."
  cd "$ECO_DIR"
  docker compose up --build -d
  sleep 5
  echo "✅ EcoCycle listo"
  echo "   Backend:  http://localhost:5000"
  echo "   Frontend: http://localhost:4200"
}

cmd_ecocycle_stop() {
  cd "$ECO_DIR" && docker compose down 2>/dev/null || true
}

# ─── Todo ────────────────────────────────────────────────────────────────────

cmd_all() {
  setup_fb
  cmd_visor
  echo ""
  cmd_ecocycle
  echo ""
  echo "╔══════════════════════════════════════════════════╗"
  echo "║   EcoCycle - Todo corriendo                      ║"
  echo "║                                                  ║"
  echo "║   Visor YOLO:  http://localhost:8000              ║"
  echo "║   Visor API:   http://localhost:3000              ║"
  echo "║   EcoCycle:    http://localhost:5000              ║"
  echo "║   Frontend:    http://localhost:4200              ║"
  echo "╚══════════════════════════════════════════════════╝"
}

# ─── Producción (VPS) ────────────────────────────────────────────────────────

PROD_COMPOSE="$SHELL_DIR/docker-compose.prod.yml"

cmd_prod() {
  setup_fb
  echo "🚀 Desplegando EcoCycle en modo PRODUCCIÓN..."
  if [ -z "${MACHINE_API_KEY:-}" ]; then
    echo "⚠️  MACHINE_API_KEY vacía en ecocycle.env: los endpoints de máquina quedarán abiertos."
    echo "   Genera una con: openssl rand -hex 24"
  fi
  cd "$SHELL_DIR"
  docker compose -f "$PROD_COMPOSE" up --build -d
  echo ""
  echo "✅ Producción desplegada (puertos solo en 127.0.0.1, expuestos vía nginx)"
  echo "   Frontend: 127.0.0.1:4200  →  nginx /"
  echo "   Backend:  127.0.0.1:5000  →  nginx /api/"
  echo "   Visor:    127.0.0.1:3000  →  nginx /visor/"
  echo ""
  echo "   Nginx: copiar scripts/nginx-proxy.conf a /etc/nginx/sites-available/ecocycle"
}

cmd_prod_stop() {
  cd "$SHELL_DIR" && docker compose -f "$PROD_COMPOSE" down 2>/dev/null || true
  echo "✅ Producción detenida"
}

cmd_prod_logs() {
  cd "$SHELL_DIR" && docker compose -f "$PROD_COMPOSE" logs -f --tail=100
}

# ─── Otros ───────────────────────────────────────────────────────────────────

cmd_yolo() {
  echo "🧠 Iniciando YOLO standalone..."
  cd "$VISOR_DIR"
  if [ -f .venv/bin/python ]; then
    .venv/bin/python yolo_service.py
  else
    python3 yolo_service.py
  fi
}

cmd_simulate() {
  echo "🎬 Simulando flujo completo (avanzado)..."
  cd "$SHELL_DIR"
  python3 scripts/simulate_flow.py "$@"
}

cmd_flow() {
  IMAGE="${1:-fotoreal.jpg}"
  BOTELLAS="${2:-2}"
  echo "╔══════════════════════════════════════════════╗"
  echo "║  QR → IoT → Puntos                          ║"
  echo "║  Imagen: $IMAGE                             "
  echo "║  Botellas: $BOTELLAS                        "
  echo "╚══════════════════════════════════════════════╝"
  cd "$SHELL_DIR"
  python3 scripts/simulate_flow.py --image "$IMAGE" --botellas "$BOTELLAS" --clean
}

cmd_test() {
  echo "🧪 Probando detección Visor..."
  if [ -f "$VISOR_DIR/descarga.jpeg" ]; then
    curl -s -X POST http://localhost:3000/detect \
      -F "image=@$VISOR_DIR/descarga.jpeg" | python3 -m json.tool 2>/dev/null || \
    curl -s -X POST http://localhost:3000/detect \
      -F "image=@$VISOR_DIR/descarga.jpeg"
  else
    echo "❌ No se encontró imagen de prueba"
  fi
}

cmd_genconfig() {
  echo "🔧 Generando configs desde ecocycle.env..."
  cd "$SHELL_DIR"
  python3 scripts/generate_configs.py
}

cmd_logs() {
  echo "📋 Mostrando logs de todos los servicios..."
  cd "$VISOR_DIR" && docker compose logs --tail=20 2>/dev/null || true
  cd "$ECO_DIR" && docker compose logs --tail=20 2>/dev/null || true
}

cmd_stop() {
  cmd_visor_stop
  cmd_ecocycle_stop
  echo "✅ Todo detenido"
}

cmd_clean() {
  cmd_stop
  echo "🧹 Limpiando..."
  rm -rf "$VISOR_DIR/dist" "$VISOR_DIR/debug_images/"*.jpg 2>/dev/null || true
  echo "✅ Limpio"
}

cmd_help() {
  echo "╔══════════════════════════════════════════════════╗"
  echo "║   EcoCycle - Script de orquestación              ║"
  echo "╠══════════════════════════════════════════════════╣"
  echo "║  Uso: ./run.sh <comando>                         ║"
  echo "╠══════════════════════════════════════════════════╣"
  echo "║  all           Inicia TODO en modo desarrollo    ║"
  echo "║  prod          Despliegue de producción (VPS)    ║"
  echo "║  prod-stop     Detener producción                ║"
  echo "║  prod-logs     Logs de producción                ║"
  echo "║  visor         Solo Visor (YOLO + NestJS)        ║"
  echo "║  ecocycle      Solo EcoCycle (.NET + Angular)    ║"
  echo "║  stop          Detener todo                      ║"
  echo "║  visor-stop    Detener Visor                     ║"
  echo "║  ecocycle-stop Detener EcoCycle                  ║"
  echo "║  logs          Ver logs de todos                 ║"
echo "║  yolo          YOLO standalone (local)           ║"
echo "║  flow [img] [n]  QR→IoT→puntos (n botellas, default 2)║"
echo "║  simulate     [args]  Simulación avanzada        ║"
echo "║  test          Probar detección Visor            ║"
echo "║  genconfig     Regenerar configs desde ecocycle.env║"
  echo "║  setup-fb      Crear placeholders Firebase       ║"
  echo "║  clean         Limpiar compilados                ║"
  echo "║  help          Mostrar esta ayuda                ║"
  echo "╚══════════════════════════════════════════════════╝"
  echo ""
  echo "📱 Apps Android (Android Studio):"
  echo "   Tablet:  maquina_EcoCycle/"
  echo "   Móvil:   EcoCycle-Movil/"
  echo ""
  echo "🔌 ESP32-CAM (Arduino IDE):"
  echo "   Sketch:  CodigoArduino/sketch_jun26a_copy_*/"
  echo ""
  echo "⚙️  Configuración centralizada:"
  echo "   Editar ecocycle.env y luego: ./run.sh genconfig"
  echo ""
  echo "🔥 Firebase:"
  echo "   ./run.sh setup-fb"
  echo "   Luego descargar clave desde Firebase Console →"
  echo "   Configuración → Cuentas de servicio → Generar clave"
  echo "   Guardar en:"
  echo "     Visor/firebase-service-account.json"
  echo "     EcoCycle/backend/Back/firebase-key.json"
}

case "${1:-help}" in
  all)           cmd_all ;;
  prod)          cmd_prod ;;
  prod-stop)     cmd_prod_stop ;;
  prod-logs)     cmd_prod_logs ;;
  visor)         cmd_visor ;;
  ecocycle)      cmd_ecocycle ;;
  stop)          cmd_stop ;;
  visor-stop)    cmd_visor_stop ;;
  ecocycle-stop) cmd_ecocycle_stop ;;
  logs)          cmd_logs ;;
  yolo)          cmd_yolo ;;
  flow)          shift 2>/dev/null; cmd_flow "$@" ;;
  simulate)      shift; cmd_simulate "$@" ;;
  test)          cmd_test ;;
  genconfig)     cmd_genconfig ;;
  setup-fb)      setup_fb ;;
  clean)         cmd_clean ;;
  help|*)        cmd_help ;;
esac
