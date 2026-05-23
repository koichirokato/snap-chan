#include "camera.h"
#include <M5Unified.h>
#include <esp_camera.h>

bool camera_initialized = false;
uint8_t* photo_buf = nullptr;
size_t photo_len = 0;

bool setupCamera() {
  // M5UnifiedのI2Cを解放してesp_cameraがGPIO11/12を使えるようにする
  M5.In_I2C.release();

  camera_config_t config = {};
  config.pin_pwdn      = -1;
  config.pin_reset     = -1;
  config.pin_xclk      = 2;
  config.pin_sccb_sda  = 12;
  config.pin_sccb_scl  = 11;
  config.pin_d7 = 47; config.pin_d6 = 48;
  config.pin_d5 = 16; config.pin_d4 = 15;
  config.pin_d3 = 42; config.pin_d2 = 41;
  config.pin_d1 = 40; config.pin_d0 = 39;
  config.pin_vsync     = 46;
  config.pin_href      = 38;
  config.pin_pclk      = 45;
  config.xclk_freq_hz  = 20000000;
  // M5SpeakerがLEDC ch0/timer0を使うためch1/timer1を使う
  config.ledc_timer    = LEDC_TIMER_1;
  config.ledc_channel  = LEDC_CHANNEL_1;
  config.pixel_format  = PIXFORMAT_RGB565;
  config.frame_size    = FRAMESIZE_QVGA;
  config.jpeg_quality  = 80;
  config.fb_count      = 2;
  config.fb_location   = CAMERA_FB_IN_PSRAM;
  config.grab_mode     = CAMERA_GRAB_LATEST;
  config.sccb_i2c_port = -1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("[CAM] init failed");
    return false;
  }

  // AWB/AEC安定のため最初の数フレームを捨てる
  for (int i = 0; i < 4; i++) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
    delay(50);
  }
  Serial.println("[CAM] ready");
  return true;
}

void takePhotoToPSRAM() {
  if (!camera_initialized) {
    Serial.println("[CAM] not initialized");
    return;
  }
  // バッファに残っている古いフレームを捨てて最新フレームを取得
  for (int i = 0; i < 2; i++) {
    camera_fb_t* old = esp_camera_fb_get();
    if (old) esp_camera_fb_return(old);
  }
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[CAM] capture failed");
    return;
  }

  uint8_t* jpg_buf = nullptr;
  size_t   jpg_len = 0;
  bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
  esp_camera_fb_return(fb);
  if (!ok || !jpg_buf) {
    Serial.println("[CAM] jpeg conversion failed");
    return;
  }

  if (photo_buf) { free(photo_buf); photo_buf = nullptr; }
  photo_buf = (uint8_t*)ps_malloc(jpg_len);
  if (photo_buf) {
    memcpy(photo_buf, jpg_buf, jpg_len);
    photo_len = jpg_len;
    Serial.printf("[CAM] saved %d bytes to PSRAM\n", jpg_len);
  }
  free(jpg_buf);
}
