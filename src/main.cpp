#include "globals.h"
#include "wifi.h"
#include "camera.h"
#include "snap.h"
#include "web_server.h"
#include <WiFi.h>

// ---- グローバルインスタンス定義 ----
Avatar avatar;
AsyncWebServer server(80);
Preferences preferences;
bool server_started = false;

// ---- シリアルコマンド ----
static String serial_cmd_buf = "";

static void handleSerialCommand(const String& cmd) {
  if (cmd == "WIFI:CLEAR") {
    clearWiFiCredentials();
    Serial.println("{\"status\":\"ok\",\"message\":\"cleared\"}");

  } else if (cmd.startsWith("WIFI:")) {
    int sep = cmd.indexOf(':', 5);
    if (sep < 0) { Serial.println("{\"error\":\"format: WIFI:ssid:pass\"}"); return; }
    String ssid = cmd.substring(5, sep); ssid.trim();
    String pass = cmd.substring(sep + 1); pass.trim();
    if (connectWiFi(ssid, pass)) {
      if (!server_started) { server.begin(); server_started = true; }
      Serial.printf("{\"status\":\"ok\",\"ip\":\"%s\"}\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("{\"status\":\"error\",\"error\":\"connection failed\"}");
    }

  } else if (cmd == "STATUS") {
    Serial.printf("{\"wifi\":%s,\"ip\":\"%s\",\"photo\":%s,\"photo_bytes\":%d,\"camera\":%s,\"psram\":%s}\n",
      wifi_connected ? "true" : "false",
      wifi_connected ? WiFi.localIP().toString().c_str() : "none",
      photo_buf ? "true" : "false",
      (int)photo_len,
      camera_initialized ? "true" : "false",
      psramFound() ? "true" : "false");

  } else {
    Serial.printf("{\"error\":\"unknown command: %s\"}\n", cmd.c_str());
  }
}

// ---- Setup / Loop ----
void setup() {
  M5.begin();
  Serial.setRxBufferSize(32768);
  Serial.begin(921600);
  delay(500);

  avatar.init();
  avatar.setExpression(Expression::Neutral);

  setupWebServer();
  setupWiFi();

  if (wifi_connected) {
    server.begin();
    server_started = true;
    Serial.printf("HTTP server: http://%s/\n", WiFi.localIP().toString().c_str());
  }

  camera_initialized = setupCamera();
  if (!camera_initialized) {
    avatar.setSpeechText("Cam Error");
    delay(2000);
    avatar.setSpeechText("");
  }

  drawStandby();
  Serial.println("Ready. Commands: WIFI:ssid:pass | WIFI:CLEAR | STATUS");
}

void loop() {
  M5.update();

  if (!snap_in_progress && M5.Touch.getCount() > 0) {
    auto t = M5.Touch.getDetail(0);
    if (t.wasPressed() && t.y >= 160) {
      snap_in_progress = true;
      runSnapCountdown();
    }
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serial_cmd_buf.length() > 0) {
        handleSerialCommand(serial_cmd_buf);
        serial_cmd_buf = "";
      }
    } else {
      serial_cmd_buf += c;
    }
  }

  delay(1);
}
