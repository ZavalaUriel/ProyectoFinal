#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "config.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       21
#define Y5_GPIO_NUM       19
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define LED_FLASH_PIN      4

#define RESP_BUF_SIZE 2048

Servo outerGateServo, innerGateServo;
const int gateClosed = 0, gateOpen = 90;

enum State { WAITING_CMD, OPENING, CAPTURING, VALIDATING };
State state = WAITING_CMD;
char activeSid[64] = "";

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EcoCycle ESP32-CAM v3 (mem-safe) ===");

  pinMode(SENSOR_IR_PIN, INPUT_PULLUP);
  pinMode(LED_FLASH_PIN, OUTPUT);
  digitalWrite(LED_FLASH_PIN, HIGH);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  outerGateServo.attach(OUTER_GATE_PIN);
  innerGateServo.attach(INNER_GATE_PIN);
  outerGateServo.write(gateClosed);
  innerGateServo.write(gateClosed);

  camera_config_t cc{};
  cc.ledc_channel = LEDC_CHANNEL_0; cc.ledc_timer = LEDC_TIMER_0;
  cc.pin_d0 = Y2_GPIO_NUM; cc.pin_d1 = Y3_GPIO_NUM;
  cc.pin_d2 = Y4_GPIO_NUM; cc.pin_d3 = Y5_GPIO_NUM;
  cc.pin_d4 = Y6_GPIO_NUM; cc.pin_d5 = Y7_GPIO_NUM;
  cc.pin_d6 = Y8_GPIO_NUM; cc.pin_d7 = Y9_GPIO_NUM;
  cc.pin_xclk = XCLK_GPIO_NUM; cc.pin_pclk = PCLK_GPIO_NUM;
  cc.pin_vsync = VSYNC_GPIO_NUM; cc.pin_href = HREF_GPIO_NUM;
  cc.pin_sccb_sda = SIOD_GPIO_NUM; cc.pin_sccb_scl = SIOC_GPIO_NUM;
  cc.pin_pwdn = PWDN_GPIO_NUM; cc.pin_reset = RESET_GPIO_NUM;
  cc.xclk_freq_hz = 20000000; cc.pixel_format = PIXFORMAT_JPEG;
  if (psramFound()) { cc.frame_size = FRAMESIZE_VGA; cc.jpeg_quality = 8; cc.fb_count = 1; }
  else { cc.frame_size = FRAMESIZE_QVGA; cc.jpeg_quality = 10; cc.fb_count = 1; }
  if (esp_camera_init(&cc) != ESP_OK) { Serial.println("Camera FAIL"); return; }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.println(" OK");
}

void ledBlink(int n, int ms) {
  for (int i = 0; i < n; i++) { digitalWrite(LED_FLASH_PIN, LOW); delay(ms); digitalWrite(LED_FLASH_PIN, HIGH); delay(ms); }
}

// ---------- HTTP helpers with fixed buffer ----------

// Returns false on error, sets body[0]=0 if no body
bool httpGet(const char* path, char* body, int maxLen) {
  body[0] = 0;
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;
  c.printf("GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, VISOR_HOST, VISOR_PORT);

  char buf[512];
  int idx = 0;
  bool headerEnd = false;
  unsigned long t = millis() + 5000;
  while (millis() < t && idx < maxLen - 1) {
    while (c.available() && idx < maxLen - 1) {
      char ch = c.read();
      if (headerEnd) { body[idx++] = ch; }
      else {
        // shift buffer to detect \r\n\r\n
        memmove(buf, buf + 1, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = ch;
        if (buf[sizeof(buf) - 1] == '\n' && buf[sizeof(buf) - 2] == '\r' && buf[sizeof(buf) - 3] == '\n' && buf[sizeof(buf) - 4] == '\r')
          headerEnd = true;
      }
    }
    delay(5);
  }
  body[idx] = 0;
  c.stop();
  return headerEnd;
}

bool httpPostJson(const char* path, const char* jsonBody) {
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;
  int len = strlen(jsonBody);
  c.printf("POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s", path, VISOR_HOST, VISOR_PORT, len, jsonBody);

  unsigned long t = millis() + 5000;
  char resp[64];
  int ri = 0;
  while (millis() < t && ri < 60) {
    while (c.available() && ri < 60) resp[ri++] = c.read();
    if (ri > 0 && resp[ri - 1] == '\n') break;
    delay(5);
  }
  resp[ri] = 0;
  c.stop();
  return strstr(resp, "200 OK") != NULL || strstr(resp, "200 ") != NULL;
}

// ---------- Gate command ----------

bool leerGateCommand(char* outSid, int sidMax) {
  outSid[0] = 0;
  char path[64];
  snprintf(path, sizeof(path), "/gate-command/%s", MACHINE_ID);

  char body[RESP_BUF_SIZE];
  if (!httpGet(path, body, sizeof(body))) return false;

  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) return false;
  if (!(doc["openOuter"] | false)) return false;

  const char* sid = doc["sessionId"];
  if (!sid || strlen(sid) == 0) return false;
  strncpy(outSid, sid, sidMax - 1);
  outSid[sidMax - 1] = 0;
  return true;
}

// ---------- Photo + YOLO ----------

bool enviarFotoAvisor(uint8_t* img, size_t len, JsonDocument& outDoc) {
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;

  const char* boundary = "--EcoCycleBound";
  int hlen = snprintf(NULL, 0,
    "--%s\r\nContent-Disposition: form-data; name=\"image\"; filename=\"cap.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n", boundary);
  int flen = snprintf(NULL, 0, "\r\n--%s--\r\n", boundary);
  int total = hlen + len + flen;

  c.printf("POST /machine-detect HTTP/1.1\r\nHost: %s:%d\r\nX-Machine-Id: %s\r\nContent-Type: multipart/form-data; boundary=%s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
    VISOR_HOST, VISOR_PORT, MACHINE_ID, boundary, total);
  c.printf("--%s\r\nContent-Disposition: form-data; name=\"image\"; filename=\"cap.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n", boundary);
  c.write(img, len);
  c.printf("\r\n--%s--\r\n", boundary);

  unsigned long t = millis() + 15000;
  char body[RESP_BUF_SIZE];
  int idx = 0;
  char hbuf[8] = {0};
  bool headers = false;
  while (millis() < t && idx < RESP_BUF_SIZE - 1) {
    while (c.available() && idx < RESP_BUF_SIZE - 1) {
      char ch = c.read();
      if (headers) { body[idx++] = ch; }
      else {
        memmove(hbuf, hbuf + 1, 7);
        hbuf[7] = ch;
        if (hbuf[7] == '\n' && hbuf[6] == '\r' && hbuf[5] == '\n' && hbuf[4] == '\r') headers = true;
      }
    }
    delay(5);
    if (!c.connected() && !c.available()) break;
  }
  body[idx] = 0;
  c.stop();

  return deserializeJson(outDoc, body) == DeserializationError::Ok;
}

// ---------- Servo actions ----------

void openOuterGate() {
  Serial.println("OUTER OPEN (bottle enters)");
  outerGateServo.write(gateOpen);
  delay(3000);
  outerGateServo.write(gateClosed);
  delay(1000);
}

void openInnerGate() {
  Serial.println("INNER OPEN (stored)");
  ledBlink(3, 150);
  innerGateServo.write(gateOpen);
  delay(3000);
  innerGateServo.write(gateClosed);
  delay(500);
}

void returnObject() {
  Serial.println("OUTER OPEN (rejected)");
  ledBlink(1, 300);
  outerGateServo.write(gateOpen);
  delay(3000);
  outerGateServo.write(gateClosed);
  delay(500);
}

// ---------- Main loop ----------

void loop() {
  switch (state) {
    case WAITING_CMD: {
      char sid[64];
      if (leerGateCommand(sid, sizeof(sid))) {
        strcpy(activeSid, sid);
        Serial.printf("CMD: open gate for %s\n", activeSid);
        state = OPENING;
      }
      delay(2000);
      break;
    }

    case OPENING:
      openOuterGate();
      state = CAPTURING;
      break;

    case CAPTURING: {
      Serial.println("Capturing...");
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) { Serial.println("Camera FAIL"); state = VALIDATING; break; }

      JsonDocument doc;
      bool detected = enviarFotoAvisor(fb->buf, fb->len, doc);
      esp_camera_fb_return(fb);

      bool esBotella = detected && (doc["botella"] | false);
      Serial.printf("YOLO: %s\n", esBotella ? "BOTTLE" : "NOT BOTTLE");

      // Confirm to Visor
      char json[256];
      snprintf(json, sizeof(json),
        "{\"sessionId\":\"%s\",\"machineId\":\"%s\",\"esBotella\":%s}",
        activeSid, MACHINE_ID, esBotella ? "true" : "false");
      httpPostJson("/machine-confirm", json);

      if (esBotella) openInnerGate();
      else returnObject();

      activeSid[0] = 0;
      state = VALIDATING;
      break;
    }

    case VALIDATING:
      delay(2000);
      state = WAITING_CMD;
      break;
  }
}
