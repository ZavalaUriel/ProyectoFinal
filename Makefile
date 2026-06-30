.PHONY: all visor visor-dev yolo backend clean setup-fb

# Proyecto EcoCycle - Makefile para orquestar servicios
# ============================================================

SHELL := /bin/bash
NVM_CMD := export NVM_DIR="$$HOME/.nvm" && [ -s "$$NVM_DIR/nvm.sh" ] && . "$$NVM_DIR/nvm.sh"
VISOR_DIR := $(CURDIR)/Visor
DOCKER := docker compose -f $(VISOR_DIR)/docker-compose.yml

# ─── Todo en uno ────────────────────────────────────────────────────────────
all: setup-fb visor
	@echo ""
	@echo "╔══════════════════════════════════════════════════╗"
	@echo "║   EcoCycle - Todo corriendo                      ║"
	@echo "║                                                  ║"
	@echo "║   YOLO:    http://localhost:8000                  ║"
	@echo "║   Visor:   http://localhost:3000                  ║"
	@echo "║   Docs:    Visor/.docs/README.md                  ║"
	@echo "╚══════════════════════════════════════════════════╝"

# ─── Crear placeholder de Firebase ───────────────────────────────────────────
setup-fb:
	@if [ ! -f "$(VISOR_DIR)/firebase-service-account.json" ]; then \
		echo '{"type":"service_account","project_id":"ecocycle-e9c04","private_key_id":"placeholder","private_key":"placeholder","client_email":"placeholder@placeholder.com","client_id":"0","auth_uri":"https://accounts.google.com/o/oauth2/auth","token_uri":"https://oauth2.googleapis.com/token"}' > "$(VISOR_DIR)/firebase-service-account.json"; \
		echo "⚠️  Creado placeholder firebase-service-account.json"; \
		echo "   Reemplázalo con tu clave real desde Firebase Console →"; \
		echo "   Configuración → Cuentas de servicio → Generar clave"; \
	fi

# ─── Visor completo (YOLO + NestJS) con Docker ──────────────────────────────
visor: setup-fb
	@echo "🚀 Iniciando Visor (YOLO + NestJS) con Docker..."
	$(DOCKER) up --build -d
	@echo "⏳ Esperando a que YOLO cargue el modelo..."
	@sleep 3
	@echo "✅ Visor iniciado"
	@echo "   YOLO:  http://localhost:8000"
	@echo "   Visor: http://localhost:3000"

visor-logs:
	$(DOCKER) logs -f

visor-stop:
	$(DOCKER) down

visor-restart: visor-stop visor

# ─── Modo desarrollo (sin Docker) ───────────────────────────────────────────
visor-dev: setup-fb
	@echo "🚀 Iniciando Visor en modo desarrollo..."
	@echo "📦 Asegúrate de tener las dependencias instaladas:"
	@echo "   cd Visor && pip install -r requirements.txt 2>/dev/null || pip install ultralytics Pillow"
	@echo "   cd Visor && npm install --legacy-peer-deps"
	@echo ""
	@echo "   Luego en terminales separadas:"
	@echo "   Terminal 1:  cd Visor && python3 yolo_service.py"
	@echo "   Terminal 2:  cd Visor && npm run start:dev"
	@echo ""
	@echo "   O usa este comando para YOLO:"
	@echo ""
	@$(MAKE) yolo & (sleep 4 && $(MAKE) backend)

yolo:
	@echo "🧠 Iniciando YOLO en puerto 8000..."
	@cd $(VISOR_DIR) && \
	 $(NVM_CMD) && \
	 .venv/bin/python yolo_service.py 2>/dev/null || \
	 python3 yolo_service.py 2>/dev/null || \
	 echo "❌ Error: instala ultralytics: pip install ultralytics Pillow"

backend: setup-fb
	@echo "🌐 Iniciando NestJS en puerto 3000..."
	@cd $(VISOR_DIR) && \
	 $(NVM_CMD) && \
	 YOLO_HOST=localhost YOLO_PORT=8000 npm run start:dev

# ─── Build NestJS ──────────────────────────────────────────────────────────
build:
	@echo "📦 Compilando NestJS..."
	@cd $(VISOR_DIR) && $(NVM_CMD) && npm run build
	@echo "✅ Compilación exitosa"

# ─── Probar detección ──────────────────────────────────────────────────────
test-detect:
	@echo "🧪 Probando detección con imagen de prueba..."
	@curl -s -X POST http://localhost:3000/detect \
	 -F "image=@$(VISOR_DIR)/descarga.jpeg" | python3 -m json.tool 2>/dev/null || \
	 curl -s -X POST http://localhost:3000/detect \
	 -F "image=@$(VISOR_DIR)/descarga.jpeg"

# ─── Limpieza ──────────────────────────────────────────────────────────────
clean: visor-stop
	@echo "🧹 Limpiando..."
	@rm -rf $(VISOR_DIR)/dist $(VISOR_DIR)/debug_images/*.jpg
	@echo "✅ Limpio"

clean-all: clean
	@rm -rf $(VISOR_DIR)/node_modules $(VISOR_DIR)/.venv
	@echo "✅ Todo eliminado (node_modules, venv, dist)"

# ─── Ayuda ─────────────────────────────────────────────────────────────────
help:
	@echo "╔══════════════════════════════════════════════════╗"
	@echo "║   EcoCycle - Comandos disponibles                ║"
	@echo "╠══════════════════════════════════════════════════╣"
	@echo "║  make all        → Inicia todo (recomendado)     ║"
	@echo "║  make visor      → YOLO + NestJS con Docker      ║"
	@echo "║  make visor-dev  → Modo desarrollo (sin Docker)  ║"
	@echo "║  make yolo       → Solo servicio YOLO            ║"
	@echo "║  make backend    → Solo NestJS backend           ║"
	@echo "║  make build      → Compilar NestJS               ║"
	@echo "║  make test-detect→ Probar con imagen de ejemplo  ║"
	@echo "║  make visor-logs → Ver logs de Docker            ║"
	@echo "║  make visor-stop → Detener servicios             ║"
	@echo "║  make setup-fb   → Crear placeholder Firebase    ║"
	@echo "║  make clean      → Limpiar dist y debug          ║"
	@echo "╚══════════════════════════════════════════════════╝"
	@echo ""
	@echo "📱 Apps Android (compilar con Android Studio):"
	@echo "   Tablet: maquina_EcoCycle/"
	@echo "   Móvil:  EcoCycle-Movil/"
	@echo ""
	@echo "🔌 ESP32-CAM: Subir sketch desde Arduino IDE"
	@echo "   Ruta: CodigoArduino/sketch_jun26a_copy_*/"
	@echo ""
	@echo "🔥 Firebase: Configurar clave de servicio"
	@echo "   Descargar en: Firebase Console → Configuración"
	@echo "   → Cuentas de servicio → Generar clave privada"
	@echo "   Guardar como: Visor/firebase-service-account.json"
