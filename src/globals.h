#pragma once
#include <M5Unified.h>
#include <Avatar.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

using namespace m5avatar;

extern Avatar avatar;
extern AsyncWebServer server;
extern Preferences preferences;
extern bool server_started;
