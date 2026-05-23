#include "web_server.h"
#include "globals.h"
#include "camera.h"

static const char snapchan_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>SnapChan</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: #1a1a2e;
    color: #eee;
    font-family: -apple-system, BlinkMacSystemFont, sans-serif;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 24px 16px;
    gap: 20px;
  }
  h1 { font-size: 1.6rem; letter-spacing: 0.05em; color: #f0c040; }
  .photo-wrap {
    width: 100%;
    max-width: 480px;
    border-radius: 12px;
    overflow: hidden;
    background: #111;
    border: 2px solid #333;
  }
  img { width: 100%; display: block; }
  .caption { font-size: 0.8rem; color: #888; text-align: center; padding: 8px; }
  button {
    padding: 14px 40px;
    font-size: 1rem;
    font-weight: bold;
    background: #f0c040;
    color: #111;
    border: none;
    border-radius: 50px;
    cursor: pointer;
    width: 100%;
    max-width: 320px;
  }
  button:active { opacity: 0.75; }
</style>
</head>
<body>
<h1>SnapChan</h1>
<div class="photo-wrap">
  <img id="photo" alt="スタックチャン視点の写真">
  <div class="caption">スタックチャン視点</div>
</div>
<button onclick="dl()">ダウンロード</button>
<script>
  const img = document.getElementById('photo');
  img.src = '/photo.jpg?t=' + Date.now();
  img.onerror = () => { img.alt = 'まだ写真がないよ！スタックチャンをタッチしてみて'; };
  function dl() {
    const a = document.createElement('a');
    a.href = '/photo.jpg?t=' + Date.now();
    a.download = 'snapchan.jpg';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
  }
</script>
</body>
</html>
)rawliteral";

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse* res = request->beginResponse(
      200, "text/html; charset=utf-8", snapchan_html);
    res->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(res);
  });

  server.on("/photo.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!photo_buf || photo_len == 0) {
      request->send(404, "application/json", "{\"error\":\"no photo yet\"}");
      return;
    }
    uint8_t* buf = photo_buf;
    size_t   len = photo_len;
    AsyncWebServerResponse* res = request->beginChunkedResponse(
      "image/jpeg",
      [buf, len](uint8_t* out, size_t maxLen, size_t index) -> size_t {
        if (index >= len) return 0;
        size_t n = len - index;
        if (n > maxLen) n = maxLen;
        memcpy(out, buf + index, n);
        return n;
      }
    );
    res->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    res->addHeader("Access-Control-Allow-Origin", "*");
    request->send(res);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "not found");
  });
}
