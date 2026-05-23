#pragma once
#include <Arduino.h>

extern bool camera_initialized;
extern uint8_t* photo_buf;
extern size_t photo_len;

bool setupCamera();
void takePhotoToPSRAM();
