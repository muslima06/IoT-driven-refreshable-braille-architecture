#ifndef CAMERA_SERVER_H
#define CAMERA_SERVER_H

// Start the camera HTTP server (web UI on port 80, MJPEG stream on port 81).
// Must be called after esp_camera_init() has succeeded.
void startCameraServer();

// Configure LED flash pin for illumination control.
void setupLedFlash(int pin);

#endif
