#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "config.h"

// Pines AI Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LED flash / indicador (GPIO 4, activo en LOW)
#define LED_FLASH_PIN 4

// Servos (doble compuerta)
Servo outerGateServo;   // Compuerta exterior (devuelve objeto)
Servo innerGateServo;   // Compuerta interior (deja caer botella)

const int gateClosed = 0;
const int gateOpen = 90;

// Estados de la máquina (modo prueba sin IR)
enum MachineState {
  WAITING_FOR_SESSION,
  CAPTURING,
};

MachineState state = WAITING_FOR_SESSION;
String activeSessionId = "";

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EcoCycle ESP32-CAM Iniciando ===");

  // Sensores (opcional, no necesario para prueba)
  pinMode(SENSOR_IR_PIN, INPUT_PULLUP);

  // LED flash
  pinMode(LED_FLASH_PIN, OUTPUT);
  digitalWrite(LED_FLASH_PIN, HIGH); // apagado (activo LOW)

  Serial.println("⚠️  Modo prueba sin IR - esperando sesión activa...");

  // Servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  outerGateServo.attach(OUTER_GATE_PIN);
  innerGateServo.attach(INNER_GATE_PIN);
  outerGateServo.write(gateClosed);
  innerGateServo.write(gateClosed);
  Serial.println("Servos inicializados (compuertas cerradas)");

  // Cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) { config.frame_size = FRAMESIZE_VGA; config.jpeg_quality = 8; config.fb_count = 2; }
  else { config.frame_size = FRAMESIZE_QVGA; config.jpeg_quality = 10; config.fb_count = 1; }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error cámara: 0x%x\n", err);
    return;
  }
  Serial.println("Cámara inicializada");

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi conectado.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void parpadearLed(int veces, int duracion) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_FLASH_PIN, LOW);
    delay(duracion);
    digitalWrite(LED_FLASH_PIN, HIGH);
    delay(duracion);
  }
}

void loop() {
  switch (state) {
    case WAITING_FOR_SESSION:
      // Modo prueba: buscar sesión activa cada 5 segundos
      activeSessionId = getActiveSessionFromVisor();
      if (activeSessionId.length() > 0) {
        Serial.printf("✅ Sesión activa encontrada: %s\n", activeSessionId.c_str());
        state = CAPTURING;
      } else {
        Serial.println("⌛ Sin sesión activa, reintentando en 5s...");
        delay(5000);
      }
      break;

    case CAPTURING:
      Serial.println("📸 Capturando y enviando foto...");
      tomarFotoYEnviar();
      Serial.println("⏳ Esperando 10s antes de siguiente detección...");
      delay(10000);
      // Volver a leer estado de la sesión
      state = WAITING_FOR_SESSION;
      break;

    default:
      state = WAITING_FOR_SESSION;
      break;
  }
}

String getActiveSessionFromVisor() {
  WiFiClient client;
  if (!client.connect(VISOR_HOST, VISOR_PORT)) {
    Serial.println("Error conectando al Visor");
    return "";
  }

  client.printf("GET /active-session/%s HTTP/1.1\r\n", MACHINE_ID);
  client.printf("Host: %s:%d\r\n", VISOR_HOST, VISOR_PORT);
  client.print("Connection: close\r\n\r\n");

  unsigned long timeout = millis() + 5000;
  String response = "";
  while (millis() < timeout) {
    while (client.available()) {
      response += (char)client.read();
    }
    if (response.indexOf("\r\n\r\n") >= 0) break;
    delay(10);
  }
  client.stop();

  int bodyStart = response.indexOf("\r\n\r\n");
  if (bodyStart < 0) return "";

  String body = response.substring(bodyStart + 4);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.printf("Error parseando JSON: %s\n", error.c_str());
    return "";
  }

  const char* sid = doc["sessionId"];
  if (sid != nullptr && strlen(sid) > 0 && strcmp(sid, "null") != 0) {
    return String(sid);
  }
  return "";
}

void tomarFotoYEnviar() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error capturando foto");
    return;
  }

  bool esBotella = enviarFotoAvisor(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (esBotella) {
    Serial.println("BOTELLA DETECTADA - Abriendo compuerta interna");
    parpadearLed(3, 150); // 3 parpadeos rápidos = botella ok
    abrirCompuertaInterna();
  } else {
    Serial.println("NO ES BOTELLA - Devolviendo objeto");
    parpadearLed(1, 300); // 1 parpadeo largo = rechazado
    devolverObjeto();
  }
}

bool enviarFotoAvisor(uint8_t* image_data, size_t image_size) {
  WiFiClient client;
  if (!client.connect(VISOR_HOST, VISOR_PORT)) {
    Serial.println("Error conectando al Visor");
    return false;
  }

  String boundary = "----EcoCycleBoundary";
  String header = "--" + boundary + "\r\n";
  header += "Content-Disposition: form-data; name=\"image\"; filename=\"machine_001_capture.jpg\"\r\n";
  header += "Content-Type: image/jpeg\r\n\r\n";

  String footer = "\r\n--" + boundary + "--\r\n";
  int totalLength = header.length() + image_size + footer.length();

  client.printf("POST /machine-detect HTTP/1.1\r\n");
  client.printf("Host: %s:%d\r\n", VISOR_HOST, VISOR_PORT);
  client.printf("X-Machine-Id: %s\r\n", MACHINE_ID);
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary.c_str());
  client.printf("Content-Length: %d\r\n", totalLength);
  client.print("Connection: close\r\n\r\n");

  client.print(header);
  uint8_t* p = image_data;
  size_t remaining = image_size;
  while (remaining > 0) {
    size_t chunk = remaining < 1024 ? remaining : 1024;
    client.write(p, chunk);
    p += chunk;
    remaining -= chunk;
  }
  client.print(footer);

  unsigned long timeout = millis() + 15000;
  String response = "";
  while (millis() < timeout) {
    while (client.available()) {
      response += (char)client.read();
    }
    if (response.indexOf("\r\n\r\n") >= 0) {
      unsigned long bodyTimeout = millis() + 3000;
      while (millis() < bodyTimeout) {
        while (client.available()) {
          response += (char)client.read();
        }
        delay(10);
        if (!client.connected()) break;
      }
      break;
    }
    delay(10);
  }
  client.stop();

  int httpCode = 0;
  if (response.startsWith("HTTP/1.1 ")) {
    httpCode = response.substring(9, 12).toInt();
  }

  int bodyStart = response.indexOf("\r\n\r\n");
  if (httpCode < 200 || httpCode >= 300 || bodyStart < 0) {
    Serial.printf("Error HTTP: %d\n", httpCode);
    return false;
  }

  String body = response.substring(bodyStart + 4);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.printf("Error parseando JSON respuesta: %s\n", error.c_str());
    return false;
  }

  bool botella = doc["botella"] | false;
  return botella;
}

void abrirCompuertaInterna() {
  innerGateServo.write(gateOpen);
  delay(3000);
  innerGateServo.write(gateClosed);
  delay(500);
}

void devolverObjeto() {
  outerGateServo.write(gateOpen);
  delay(3000);
  outerGateServo.write(gateClosed);
  delay(500);
}
