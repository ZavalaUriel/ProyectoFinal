// EcoCycle ESP32 v5 - Solo compuerta (sin cámara, sin segunda validación)
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include "config.h"

Servo outerGate;

#define LED_PIN 4
#define BUF 2048
char g_path[80];
char g_body[BUF];

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n=== EcoCycle ESP32 v5 (solo compuerta) ===\n");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  ESP32PWM::allocateTimer(2);
  outerGate.attach(OUTER_GATE_PIN);
  outerGate.write(0);
  Serial.println("Servo OK");

  Serial.print("WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int w = 0;
  while (WiFi.status() != WL_CONNECTED && w < 20) {
    delay(1000);
    Serial.print('.');
    w++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" FAIL");
  }

  Serial.println("Setup OK\n");
}

bool wifiOk() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    int r = 0;
    while (WiFi.status() != WL_CONNECTED && r < 10) {
      delay(500);
      r++;
    }
  }
  return WiFi.status() == WL_CONNECTED;
}

bool apiGet(const char* path, char* out, int maxLen) {
  out[0] = 0;
  if (!wifiOk()) return false;
  WiFiClient c;
  if (!c.connect(VISOR_HOST, VISOR_PORT)) return false;
  c.printf("GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, VISOR_HOST, VISOR_PORT);
  char buf[8] = { 0 };
  int i = 0;
  bool hdr = false;
  unsigned long t = millis() + 8000;
  while (millis() < t && i < maxLen - 1) {
    while (c.available() && i < maxLen - 1) {
      char ch = c.read();
      if (hdr) {
        out[i++] = ch;
      } else {
        memmove(buf, buf + 1, 7);
        buf[7] = ch;
        if (buf[7] == '\n' && buf[6] == '\r' && buf[5] == '\n' && buf[4] == '\r') hdr = true;
      }
    }
    delay(5);
  }
  out[i] = 0;
  c.stop();
  return hdr && i > 0;
}

void loop() {
  delay(2000);

  if (!wifiOk()) {
    Serial.println("Sin WiFi, esperando...");
    return;
  }

  snprintf(g_path, sizeof(g_path), "/gate-command/%s", MACHINE_ID);
  if (!apiGet(g_path, g_body, BUF)) return;

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, g_body) != DeserializationError::Ok) return;

  if (!(doc["openOuter"] | false)) return;

  const char* sid = doc["sessionId"];
  if (!sid || strlen(sid) == 0) return;
  Serial.printf("Compuerta: abriendo (session=%s)\n", sid);

  digitalWrite(LED_PIN, HIGH);
  outerGate.write(90);
  delay(3000);
  outerGate.write(0);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Compuerta cerrada\n");
}
