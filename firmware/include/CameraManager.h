#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include "esp_camera.h"

class CameraManager {
public:
    static bool init();
    static camera_fb_t* capture();
    static void release(camera_fb_t* fb);
};

#endif
