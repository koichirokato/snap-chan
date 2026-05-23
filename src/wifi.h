#pragma once
#include <Arduino.h>

extern bool wifi_connected;

void setupWiFi();
bool connectWiFi(const String& ssid, const String& pass);
void saveWiFiCredentials(const String& ssid, const String& pass);
void clearWiFiCredentials();
