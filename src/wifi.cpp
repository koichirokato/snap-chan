#include "wifi.h"
#include "globals.h"
#include <WiFi.h>

bool wifi_connected = false;

static String wifi_ssid = "";
static String wifi_pass = "";

void saveWiFiCredentials(const String& ssid, const String& pass) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
}

void clearWiFiCredentials() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  wifi_ssid = "";
  wifi_pass = "";
}

bool connectWiFi(const String& ssid, const String& pass) {
  wifi_ssid = ssid;
  wifi_pass = pass;
  avatar.setSpeechText("WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    WiFi.setSleep(false);
    saveWiFiCredentials(ssid, pass);
    avatar.setSpeechText(WiFi.localIP().toString().c_str());
    delay(2000);
    avatar.setSpeechText("");
    return true;
  }
  avatar.setSpeechText("WiFi Failed");
  delay(1000);
  avatar.setSpeechText("");
  return false;
}

void setupWiFi() {
  preferences.begin("wifi", true);
  wifi_ssid = preferences.getString("ssid", "");
  wifi_pass = preferences.getString("pass", "");
  preferences.end();

  if (wifi_ssid.isEmpty()) {
    avatar.setSpeechText("WIFI:ssid:pass");
    delay(2000);
    avatar.setSpeechText("");
    return;
  }
  connectWiFi(wifi_ssid, wifi_pass);
}
