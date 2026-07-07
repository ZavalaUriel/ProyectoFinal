// EcoCycle ESP32-CAM Firmware v4 - Flujo completo
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "config.h"

// Pines cámara AI-Thinker
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
#define LED_PIN            4

Servo outerGate, innerGate;
char activeSession[64] = "";

#define BUF 2048
char g_path[80];
char g_body[BUF];
char g_resp[BUF];
char g_json[256];

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n=== EcoCycle ESP32-CAM v4 ===\n");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Cámara primero (timer 0)
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
    Serial.printf("Camara FAIL: 0x%x\n", err);
  } else {
    Serial.println("Camara OK");
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
      s->set_brightness(s, 1);
      s->set_contrast(s, 1);
      s->set_saturation(s, 1);
      s->set_quality(s, 6);
      s->set_hmirror(s, 0);
      s->set_vflip(s, 0);
      Serial.println("Sensor tuning: brightness=1, contrast=1, quality=6");
    }
  }

  // Servos (timers 2 y 3 para evitar conflicto con camara)
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  outerGate.attach(OUTER_GATE_PIN);
  innerGate.attach(INNER_GATE_PIN);
  outerGate.write(0);
  innerGate.write(0);
  Serial.println("Servos OK");

  // WiFi
  Serial.print("WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int w = 0;
  while (WiFi.status() != WL_CONNECTED && w < 20) {
    delay(1000); Serial.print('.'); w++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" FAIL");
  }

  digitalWrite(LED_PIN, LOW);
  Serial.println("Setup OK\n");
}

bool wifiOk() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    int r = 0;
    while (WiFi.status() != WL_CONNECTED && r < 10) { delay(500); r++; }
  }
  return WiFi.status() == WL_CONNECTED;
}

// HTTP GET a Visor, extrae body JSON
bool apiGet(const char* path, char* out, int maxLen) {
  out[0] = 0;
  if (!wifiOk()) return false;
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;
  c.printf("GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, VISOR_HOST, VISOR_PORT);
  char buf[8] = {0};
  int i = 0;
  bool hdr = false;
  unsigned long t = millis() + 8000;
  while (millis() < t && i < maxLen - 1) {
    while (c.available() && i < maxLen - 1) {
      char ch = c.read();
      if (hdr) { out[i++] = ch; }
      else { memmove(buf, buf+1, 7); buf[7]=ch; if (buf[7]=='\n'&&buf[6]=='\r'&&buf[5]=='\n'&&buf[4]=='\r') hdr=true; }
    }
    delay(5);
  }
  out[i]=0; c.stop();
  return hdr && i > 0;
}

// HTTP POST body JSON, retorna true si 200
bool apiPost(const char* path, const char* body) {
  if (!wifiOk()) return false;
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;
  int len = strlen(body);
  c.printf("POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
    path, VISOR_HOST, VISOR_PORT, len, body);
  unsigned long t = millis() + 8000;
  char r[64]; int ri = 0;
  while (millis() < t && ri < 60) { while (c.available() && ri < 60) r[ri++] = c.read(); delay(5); }
  r[ri] = 0; c.stop();
  return strstr(r, "200 OK") != nullptr;
}

// Enviar foto directo a YOLO
bool enviarFoto(const uint8_t* img, size_t len, char* outResult, int outMax) {
  outResult[0] = 0;
  if (!wifiOk()) return false;
  WiFiClient c;
  if (!c.connect(VISOR_HOST, 8000)) return false; // YOLO port

  c.printf("POST /detect HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
    VISOR_HOST, 8000, len);
  c.write(img, len);

  // Leer respuesta
  char hb[8] = {0};
  int ri = 0;
  bool hdr = false;
  unsigned long t = millis() + 15000;
  while (millis() < t && ri < outMax - 1) {
    while (c.available() && ri < outMax - 1) {
      char ch = c.read();
      if (hdr) { outResult[ri++] = ch; }
      else { memmove(hb, hb+1, 7); hb[7]=ch; if (hb[7]=='\n'&&hb[6]=='\r'&&hb[5]=='\n'&&hb[4]=='\r') hdr=true; }
    }
    delay(5);
  }
  outResult[ri] = 0; c.stop();
  return hdr;
}

void parpadear(int veces) {
  for (int i = 0; i < veces; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  delay(3000);

  if (!wifiOk()) {
    Serial.println("Sin WiFi, esperando...");
    return;
  }

  // 1. Consultar gate-command
  snprintf(g_path, sizeof(g_path), "/gate-command/%s", MACHINE_ID);
  if (!apiGet(g_path, g_body, BUF)) return;

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, g_body) != DeserializationError::Ok) return;

  if (!(doc["openOuter"] | false)) return;

  const char* sid = doc["sessionId"];
  if (!sid || strlen(sid) == 0) return;
  strncpy(activeSession, sid, sizeof(activeSession) - 1);
  Serial.printf("CMD recibido, session=%s\n", activeSession);

  // 2. Abrir compuerta exterior
  Serial.println("Abriendo exterior...");
  digitalWrite(LED_PIN, HIGH);
  parpadear(2);
  outerGate.write(90);
  delay(3000);
  outerGate.write(0);
  delay(1000);

  // 3. Capturar foto (flash ON)
  Serial.println("Capturando...");
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  camera_fb_t* fb = esp_camera_fb_get();
  digitalWrite(LED_PIN, LOW);
  if (!fb) {
    Serial.println("Foto FAIL");
    snprintf(g_json, sizeof(g_json), "{\"sessionId\":\"%s\",\"machineId\":\"%s\",\"esBotella\":false}", activeSession, MACHINE_ID);
    apiPost("/machine-confirm", g_json);
    digitalWrite(LED_PIN, LOW);
    return;
  }
  Serial.printf("Foto %d bytes\n", fb->len);

  // 4. Enviar a YOLO via Visor
  bool ok = enviarFoto(fb->buf, fb->len, g_resp, BUF);
  esp_camera_fb_return(fb);

  bool esBotella = false;
  if (ok) {
    StaticJsonDocument<512> rdoc;
    if (deserializeJson(rdoc, g_resp) == DeserializationError::Ok) {
      esBotella = rdoc["botella"] | false;
      Serial.printf("YOLO: %s\n", esBotella ? "BOTELLA" : "NO");
    }
  }

  // 5. Confirmar resultado
  snprintf(g_json, sizeof(g_json), "{\"sessionId\":\"%s\",\"machineId\":\"%s\",\"esBotella\":%s}",
    activeSession, MACHINE_ID, esBotella ? "true" : "false");
  apiPost("/machine-confirm", g_json);

  // 6. Acción según resultado
  if (esBotella) {
    Serial.println("Almacenando...");
    parpadear(3);
    innerGate.write(90);
    delay(3000);
    innerGate.write(0);
  } else {
    Serial.println("Devolviendo...");
    parpadear(1);
    outerGate.write(90);
    delay(3000);
    outerGate.write(0);
  }

  digitalWrite(LED_PIN, LOW);
  activeSession[0] = 0;
  Serial.println("Ciclo completo\n");
}
