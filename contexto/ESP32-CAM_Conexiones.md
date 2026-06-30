# Guía de Conexiones ESP32-CAM — EcoCycle

## Diagrama General

```
┌─────────────────────────────────────────────────────────────────┐
│                        ESP32-CAM (AI-Thinker)                    │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                     Cámara OV2640                         │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                  │
│  ┌───┐  ┌───┐  ┌──────┐  ┌──────────┐  ┌──────────────────┐   │
│  │IR │  │LED│  │Servo1│  │ Servo2   │  │  Fuente Externa   │   │
│  │13 │  │4  │  │Outer│  │ Inner    │  │  5V 2A            │   │
│  │   │  │   │  │Gate │  │ Gate     │  │                   │   │
│  │   │  │   │  │GPIO12│  │ GPIO14   │  │  ┌─ 5V ──┐        │   │
│  │   │  │   │  │      │  │          │  │  │ GND  │        │   │
│  └───┘  └───┘  └──────┘  └──────────┘  └──┴──────┴────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Lista de Componentes

| # | Componente | Cantidad | Notas |
|---|-----------|----------|-------|
| 1 | ESP32-CAM (AI-Thinker) | 1 | Microcontrolador + cámara |
| 2 | Programador FTDI (o USB-TTL) | 1 | Para cargar el código |
| 3 | Sensor Infrarrojo HC-SR501 | 1 | Detecta objeto entrante |
| 4 | Servo MG996R (o similar) | 2 | Compuerta exterior e interior |
| 5 | LED Blanco de alta luminosidad | 1 | Iluminación para la cámara |
| 6 | Resistencia 220Ω | 1 | Para el LED |
| 7 | Resistencia 10kΩ | 1 | Pull-down para sensor IR |
| 8 | Capacitor 470µF | 1 | Para estabilizar servos |
| 9 | Fuente de poder 5V 2A | 1 | Para los servos |
| 10 | Cables Dupont | Varios | Macho-hembra y hembra-hembra |

---

## Conexiones Paso a Paso

### 1. Servo 1 — Compuerta Exterior (devuelve objeto si no es botella)

| Servo MG996R | Conectar a |
|-------------|-----------|
| **Café** (GND) | GND común (ESP32 + Fuente externa) |
| **Rojo** (VCC) | **5V de la fuente externa** (NO del ESP32) |
| **Naranja** (Señal) | **GPIO 12** del ESP32-CAM |

### 2. Servo 2 — Compuerta Interior (deja caer botella al depósito)

| Servo MG996R | Conectar a |
|-------------|-----------|
| **Café** (GND) | GND común |
| **Rojo** (VCC) | **5V de la fuente externa** (misma que Servo 1) |
| **Naranja** (Señal) | **GPIO 14** del ESP32-CAM |

### 3. Sensor Infrarrojo HC-SR501

| Sensor IR | Conectar a |
|-----------|-----------|
| **GND** | GND del ESP32 |
| **VCC** | 5V (o 3.3V) del ESP32 |
| **OUT** | **GPIO 13** del ESP32-CAM |

### 4. LED de Iluminación

| LED | Conectar a |
|-----|-----------|
| **Cátodo** (-) | GND (a través de resistencia 220Ω) |
| **Ánodo** (+) | **GPIO 4** del ESP32-CAM |

### 5. Alimentación — ¡Importante!

```
╔══════════════════════════════════════════════════════════════╗
║  ⚠️  Los servos MG996R consumen MUCHA corriente (2A pico)   ║
║  NO los alimentes desde el pin 5V del ESP32-CAM             ║
║  Usa una fuente externa de 5V 2A y une las tierras (GND)   ║
╚══════════════════════════════════════════════════════════════╝
```

```
Fuente 5V 2A ──── 5V ──── Rojo Servo 1 ──── Rojo Servo 2
        │               └─── VCC Sensor IR (opcional)
        │
        └─── GND ──── Café Servo 1 ──── Café Servo 2 ──── GND ESP32
                    └─── Cátodo LED ─/\/\/─ GND (220Ω)
                    └─── GND Sensor IR
```

### 6. Programación (FTDI → ESP32-CAM)

Para cargar el código por primera vez:

| FTDI | ESP32-CAM |
|------|----------|
| 5V | 5V |
| GND | GND |
| TX | U0R (GPIO 3) |
| RX | U0T (GPIO 1) |

> **Importante:** Conectar **GPIO 0 a GND** antes de encender para entrar en modo flash. Desconectar GPIO 0 después de programar y reiniciar.

```
Para programar:   GPIO 0 → GND  (antes de encender)
Para ejecutar:    GPIO 0 → NC   (desconectado)
```

---

## Diagrama Completo

```
                         ┌─────────────────────────────┐
                         │       FUENTE 5V 2A          │
                         │  ┌─── 5V ────────┐          │
                         │  │   GND ────────┤          │
                         │  └───────────────┘          │
                         └──────────┬──────────────────┘
                                    │
        ┌───────────────────────────┼──────────────────────────┐
        │                           │                          │
        │    ┌────────────────────────────────────────────┐   │
        │    │              ESP32-CAM                     │   │
        │    │                                            │   │
        │    │  5V ───────────────────────────────────────┘   │
        │    │  GND ──────┬────────┬────────┬────────┐       │
        │    │            │        │        │        │       │
        │    │  GPIO 4 ───[LED]────┤220Ω    │        │       │
        │    │                    GND       │        │       │
        │    │  GPIO 12 ─────────────── [Servo 1 ─────┘       │
        │    │                            (Compuerta Ext.)    │
        │    │  GPIO 14 ─────────────── [Servo 2 ───────┐     │
        │    │                            (Compuerta Int.)    │
        │    │  GPIO 13 ─────────────── [Sensor IR ──────┘     │
        │    │                                            │   │
        │    │  GPIO 0 ──┐                                │   │
        │    │           ├── switch a GND (solo programar)│   │
        │    │  GND ─────┘                                │   │
        │    └────────────────────────────────────────────┘   │
        └─────────────────────────────────────────────────────┘
```

---

## Configuración del Sketch (Arduino IDE)

### Librerías necesarias

Abrir Arduino IDE → **Gestor de librerías** (Ctrl+Shift+I) e instalar:

| Librería | Buscar como | Versión |
|----------|------------|---------|
| ESP32 Servo | `ESP32Servo` | Última |
| ArduinoJson | `ArduinoJson` por Benoit Blanchon | Última |
| ESP32 Camera | Viene con el soporte de placa ESP32 | - |

### Configuración de placa

```
Placa:           AI Thinker ESP32-CAM
CPU Frequency:   240MHz (QIO)
Flash Size:      4MB
PSRAM:           Enabled
Partition Scheme: Huge APP (3MB No OTA)
Upload Speed:    115200
Port:            /dev/ttyUSB0 (o el que aparezca)
```

### Datos a configurar en el código

Editar estas líneas en el sketch:

```cpp
// Tu red WiFi
const char* ssid = "Totalplay-C8B1";
const char* password = "C8B1CBB8khASQe6W";

// IP de la computadora donde corre el Visor
const char* visorHost = "192.168.100.19";
const int visorPort = 3000;

// ID única de esta máquina
const char* machineId = "machine_001";
```

---

## Flujo de Operación

```
1. ESP32 enciende → conecta WiFi → inicializa cámara y servos
2. Espera a que el sensor IR detecte un objeto
3. Consulta al Visor cuál es la sesión activa (GET /active-session/machine_001)
4. Toma foto y la envía al Visor (POST /machine-detect)
5. Visor procesa con YOLO, incrementa contador en Firebase si es botella
6. ESP32 recibe respuesta:
   ├── botella: true  → abre Servo 2 (compuerta interior) 3s
   └── botella: false → abre Servo 1 (compuerta exterior) 3s
7. Vuelve al paso 2
```

---

## Prueba Rápida (sin sensores)

Si no tienes el sensor IR conectado y quieres probar, cambia el `loop()` del sketch a esto:

```cpp
void loop() {
  Serial.println("📸 Modo prueba - tomando foto...");
  tomarFotoYEnviar();
  delay(10000); // espera 10 segundos entre tomas
}
```
