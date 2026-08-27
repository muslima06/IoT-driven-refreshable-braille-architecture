#ifndef OCR_CLIENT_H
#define OCR_CLIENT_H

#include <Arduino.h>
#include "esp_camera.h"

class OCRClient {
public:
    static String sendImageForOCR(camera_fb_t *fb);
};

#endif
