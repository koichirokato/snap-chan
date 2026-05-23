#include "snap.h"
#include "globals.h"
#include "camera.h"
#include <WiFi.h>

volatile bool snap_in_progress = false;

void drawStandby() {
  avatar.setExpression(Expression::Neutral);
  avatar.setSpeechText("Touch to Snap!");
}

void runSnapCountdown() {
  // stop()はフラグを立てるだけでタスク終了を待たないため
  // suspend()/resume()でタスクを生かしたまま一時停止する
  avatar.suspend();

  int w = M5.Display.width();
  int h = M5.Display.height();

  M5.Display.setFont(&fonts::Font8);
  M5.Display.setTextDatum(middle_center);

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.drawString("3", w / 2, h / 2);
  delay(800);

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Display.drawString("2", w / 2, h / 2);
  delay(800);

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_RED, TFT_BLACK);
  M5.Display.drawString("1", w / 2, h / 2);
  delay(800);

  M5.Display.fillScreen(TFT_WHITE);
  delay(120);
  M5.Display.fillScreen(TFT_BLACK);

  takePhotoToPSRAM();

  avatar.resume();
  avatar.setExpression(Expression::Happy);
  avatar.setMouthOpenRatio(0.9f);
  avatar.setSpeechText("Got it!");
  delay(1500);
  avatar.setMouthOpenRatio(0.0f);

  avatar.suspend();
  M5.Display.fillScreen(TFT_BLACK);

  if (WiFi.status() != WL_CONNECTED) {
    if (photo_buf && photo_len > 0) {
      M5.Display.drawJpg(photo_buf, photo_len, 0, 0, w, h);
    }
  } else {
    int qr_size = min(w, h) - 30;
    int qr_x    = (w - qr_size) / 2;
    String url  = "http://" + WiFi.localIP().toString() + "/";
    Serial.printf("[QR] %s\n", url.c_str());
    M5.Display.qrcode(url.c_str(), qr_x, 5, qr_size, 3);
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextDatum(bottom_center);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString("Open in browser!", w / 2, h - 2);
  }

  delay(800);
  while (true) {
    M5.update();
    if (M5.Touch.getCount() > 0 && M5.Touch.getDetail(0).wasPressed()) break;
    delay(10);
  }

  avatar.resume();
  snap_in_progress = false;
  drawStandby();
}
