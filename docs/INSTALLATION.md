# Deployment and Installation

## Hardware Installation
1. Assemble the hardware on a protoboard or custom PCB.
2. Wire the solenoids to the DRV8833 module outputs.
3. Wire the ESP32 according to `WIRING.md`.

## Backend Setup
1. Install Python 3.9+.
2. Install Tesseract-OCR on your system (`apt-get install tesseract-ocr` or download Windows installer).
3. Navigate to `backend/`.
4. Run `pip install -r requirements.txt`.
5. Run the server: `python main.py`. Ensure it binds to `0.0.0.0` to be accessible on your local network.

## Firmware Setup (PlatformIO)
1. Open this project in VS Code with the PlatformIO extension installed.
2. Open `firmware/include/Config.h`.
3. Update `WIFI_SSID` and `WIFI_PASSWORD` with your credentials.
4. Update `API_URL` to point to the local IP of your Python backend.
5. Connect your ESP32-S3 via USB.
6. Click the "Upload" button in PlatformIO to compile and flash the firmware.

## Testing Procedures
1. Press the normal hardware buttons. A short beep should be heard.
2. Hold the pause button to trigger a capture. The LCD will read "Capturing..." then "Uploading...".
3. Check the Python backend logs to ensure the image is received and OCR is processing.
4. When done, a long beep will occur, and the transcribed text will begin streaming to the Braille cells.
5. Use "Next" and "Prev" to manually navigate the text.
