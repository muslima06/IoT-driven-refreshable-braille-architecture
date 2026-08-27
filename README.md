# AI-Powered OCR to Refreshable Braille Display

> An assistive technology system that captures printed text using an ESP32-S3 camera, performs OCR via a Python backend, and outputs each character as a tactile 6-dot Braille cell using solenoids — enabling visually impaired users to read printed documents independently.

---

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Project Structure](#project-structure)
- [Software Setup](#software-setup)
  - [1. Backend (Python)](#1-backend-python)
  - [2. Firmware (ESP32-S3)](#2-firmware-esp32-s3)
- [Configuration](#configuration)
- [Usage](#usage)
- [LCD Status Messages](#lcd-status-messages)
- [Firmware Modules](#firmware-modules)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)
- [License](#license)

---

## Overview

This project bridges the gap between printed text and Braille for visually impaired users. The workflow is:

1. **Capture** — The user presses the Capture button. The ESP32-S3 on-board camera takes a photo of the printed document.
2. **Upload** — The image is sent over WiFi to a local Python server running on the same network.
3. **OCR** — The server uses [Tesseract OCR](https://github.com/tesseract-ocr/tesseract) to extract text from the image.
4. **Translate** — The ESP32 receives the extracted text and converts each character into its standard 6-dot Braille pattern.
5. **Display** — Six solenoids (driven by three DRV8833 H-bridge motor drivers) physically raise/lower pins to form each Braille character.
6. **Navigate** — The user can pause, go to the previous character, or skip to the next character using dedicated buttons.

An I2C 16×2 LCD provides visual feedback (current character, status messages) for sighted assistants or debugging.

---

## System Architecture

```
┌──────────────────────────────┐        HTTP POST         ┌─────────────────────────┐
│       ESP32-S3-N16R8         │  ───── /scan ──────────► │   Python Backend        │
│  (Freenove FNK0086 Board)    │                          │   (FastAPI + Uvicorn)   │
│                              │  ◄──── {"text":"..."} ── │                         │
│  ┌────────┐  ┌────────────┐  │                          │  ┌───────────────────┐  │
│  │OV3660  │  │ 16x2 LCD   │  │                          │  │ Tesseract OCR     │  │
│  │Camera  │  │ (I2C)      │  │                          │  │ Engine            │  │
│  └────────┘  └────────────┘  │                          │  └───────────────────┘  │
│                              │                          └─────────────────────────┘
│  ┌──────────────────────────┐│
│  │ 3× DRV8833 Motor Drivers││
│  │ → 6 Solenoids (Braille) ││
│  └──────────────────────────┘│
│                              │
│  ┌──────────────────────────┐│
│  │ 4 Buttons: Capture,     ││
│  │ Pause, Prev, Next       ││
│  └──────────────────────────┘│
└──────────────────────────────┘
```

---

## Hardware Requirements

| Component | Quantity | Purpose |
|---|---|---|
| ESP32-S3-N16R8 (Freenove FNK0086) | 1 | Main controller with on-board OV3660 camera |
| DRV8833 Dual H-Bridge Motor Driver | 3 | Drive 6 solenoids (2 per driver) |
| 5V Push-Pull Solenoid | 6 | Physical Braille dot pins |
| 16×2 I2C LCD Display (address `0x27`) | 1 | Status and character display |
| Push Button | 3 | Pause, Previous, Next (Capture uses on-board BOOT button) |
| 10kΩ Resistor | 3 | External pull-ups for buttons (optional — firmware uses `INPUT_PULLUP`) |
| 5V 5A DC Power Supply | 1 | Power for ESP32, solenoids, and LCD |
| Breadboard / PCB + Jumper Wires | — | Assembly |

> **⚡ Power Note:** The solenoids draw significant current. Do **not** power them from the ESP32's 3.3V pin. Use an external 5V supply connected to the board's VBUS/5V rail, and share it with the motor drivers. Tie all grounds together.

For the complete GPIO pin mapping and wiring instructions, see [**docs/WIRING.md**](docs/WIRING.md).

---

## Project Structure

```text
AI-Powered OCR to Refreshable Braille Display/
│
├── backend/                        # Python FastAPI Backend
│   ├── __init__.py                 # Package initializer
│   ├── main.py                     # FastAPI app & /scan endpoint
│   ├── ocr_service.py              # Tesseract OCR wrapper
│   └── requirements.txt            # Python dependencies
│
├── docs/
│   ├── WIRING.md                   # Complete GPIO wiring guide
│   └── INSTALLATION.md             # Hardware assembly & deployment guide
│
├── firmware/                       # PlatformIO ESP32-S3 Firmware
│   ├── include/                    # Header files
│   │   ├── Config.h                # WiFi, API URL, and GPIO pin definitions
│   │   ├── BrailleTranslator.h     # Braille pattern lookup
│   │   ├── ButtonManager.h         # Debounced button input handler
│   │   ├── CameraManager.h         # OV3660 camera initialization & capture
│   │   ├── LCDManager.h            # I2C LCD display driver
│   │   ├── OCRClient.h             # HTTP client for server communication
│   │   ├── SolenoidController.h    # DRV8833 solenoid driver
│   │   ├── TextNavigator.h         # Text buffer and character navigation
│   │   ├── camera_index.h          # Camera web server assets
│   │   └── camera_server.h         # Camera streaming server header
│   ├── src/                        # Implementation files
│   │   ├── main.cpp                # Entry point, setup(), loop()
│   │   ├── BrailleTranslator.cpp   # A–Z → 6-dot Braille pattern mapping
│   │   ├── ButtonManager.cpp       # Debounce logic for 4 buttons
│   │   ├── CameraManager.cpp       # Camera init (SVGA resolution)
│   │   ├── LCDManager.cpp          # LCD status & character display
│   │   ├── OCRClient.cpp           # Multipart HTTP POST to backend
│   │   ├── SolenoidController.cpp  # Raise/lower dots with staggered pulses
│   │   ├── TextNavigator.cpp       # Prev/Next character traversal
│   │   └── camera_server.cpp       # Live camera stream web server
│   └── platformio.ini              # PlatformIO build configuration
│
└── README.md                       # ← You are here
```

---

## Software Setup

### 1. Backend (Python)

#### Prerequisites

- **Python 3.9+** installed on your PC
- **Tesseract OCR engine** installed on your system:
  - **Windows:** Download the 64-bit installer from [UB-Mannheim/tesseract](https://github.com/UB-Mannheim/tesseract/wiki). Install to the default path (`C:\Program Files\Tesseract-OCR\`).
  - **Linux:** `sudo apt-get install tesseract-ocr`
  - **macOS:** `brew install tesseract`

#### Steps

```bash
# 1. Navigate to the project root
cd "AI-Powered OCR to Refreshable Braille Display for Visually Impaired Users"

# 2. Create a virtual environment
python -m venv .venv

# 3. Activate it
# Windows (PowerShell):
.venv\Scripts\Activate.ps1
# Linux / macOS:
source .venv/bin/activate

# 4. Install Python dependencies
pip install -r backend/requirements.txt

# 5. Start the OCR server
.venv\Scripts\python.exe -m uvicorn backend.main:app --host 0.0.0.0 --port 8000
```

You should see:

```
INFO:     Uvicorn running on http://0.0.0.0:8000 (Press CTRL+C to quit)
```

> **Note:** The server must be accessible on your local network. Use `--host 0.0.0.0` so the ESP32 can reach it. Find your PC's local IP with `ipconfig` (Windows) or `ip addr` (Linux).

---

### 2. Firmware (ESP32-S3)

#### Prerequisites

- **VS Code** with the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode) installed
- **ESP32-S3 board** connected via USB

#### Steps

1. Open the project folder in VS Code.
2. Open `firmware/include/Config.h` and update:
   ```c
   #define WIFI_SSID     "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   #define API_URL       "http://<YOUR_PC_IP>:8000/scan"
   ```
3. Open `firmware/platformio.ini` and set `upload_port` / `monitor_port` to your ESP32's COM port (check Device Manager on Windows).
4. Click the **Upload (→)** button in the PlatformIO toolbar to compile and flash.
5. Open the **Serial Monitor** (plug icon) at `115200` baud to see boot logs and your device's IP address.

---

## Configuration

All configuration is centralized in [`firmware/include/Config.h`](firmware/include/Config.h):

| Define | Description | Example |
|---|---|---|
| `WIFI_SSID` | Your WiFi network name | `"MyHomeWiFi"` |
| `WIFI_PASSWORD` | Your WiFi password | `"password123"` |
| `API_URL` | Full URL to the backend `/scan` endpoint | `"http://192.168.0.100:8000/scan"` |
| `PIN_I2C_SDA` | I2C data pin for the LCD | `21` |
| `PIN_I2C_SCL` | I2C clock pin for the LCD | `14` |
| `PIN_DRV*_*` | GPIO pins for DRV8833 motor driver channels | See [WIRING.md](docs/WIRING.md) |
| `PIN_BTN_*` | GPIO pins for the 4 navigation buttons | See [WIRING.md](docs/WIRING.md) |

---

## Usage

### Button Controls

| Button | GPIO | Action |
|---|---|---|
| **Capture** | GPIO 0 (on-board BOOT button) | Takes a photo and sends it for OCR |
| **Pause** | GPIO 19 | Toggles auto-advance on/off |
| **Previous** | GPIO 20 | Navigate to the previous character |
| **Next** | GPIO 44 | Navigate to the next character |

### Workflow

1. **Power on** the ESP32. The LCD will show `Connecting WiFi` → `WiFi Connected!`. Next, it displays the live camera stream address (e.g., `Stream:` on line 1, and `192.168.0.x:81` on line 2) for 3 seconds, before switching to `System Ready`.
2. Place a **printed document** in front of the camera.
3. Press the **Capture** button (BOOT button on the board).
4. The LCD will show `Capturing...` → `Uploading...`.
5. Once OCR completes, the first character's Braille pattern is displayed on the solenoids.
6. Characters **auto-advance** every 2 seconds. Press **Pause** to stop, then use **Prev/Next** to navigate manually.
7. Press **Capture** again at any time to scan a new document.

---

## LCD Status Messages

| Message | Meaning |
|---|---|
| `Connecting WiFi` | Attempting to connect to the configured WiFi network |
| `WiFi Connected!` | Successfully connected to WiFi |
| `WiFi Failed` | Could not connect after 20 attempts |
| `Camera Error` | Camera module failed to initialize |
| `Stream:`<br>`<IP_ADDRESS>:81` | Displays the live camera stream IP address on boot for 3 seconds |
| `System Ready` | All systems initialized, waiting for user input |
| `Capturing...` | Camera is taking a photo |
| `Uploading...` | Image is being sent to the backend server |
| `No Text Found` | OCR completed successfully but no readable text was detected |
| `OCR Failed` | A real error occurred (network issue, server down, JSON parse failure) |
| `Cap Error` | Camera failed to capture a frame |
| `Paused` | Auto-advance is paused; use Prev/Next to navigate manually |

---

## Firmware Modules

| Module | File | Responsibility |
|---|---|---|
| **Main** | `main.cpp` | `setup()` / `loop()`, WiFi init, button polling, auto-advance logic |
| **CameraManager** | `CameraManager.cpp` | OV3660 camera initialization at SVGA resolution, frame capture and release |
| **OCRClient** | `OCRClient.cpp` | Constructs a multipart/form-data HTTP POST with the JPEG image, sends to server, parses JSON response |
| **BrailleTranslator** | `BrailleTranslator.cpp` | Maps ASCII characters (a–z, space) to standard 6-dot Braille patterns |
| **SolenoidController** | `SolenoidController.cpp` | Drives 6 solenoids via 3 DRV8833 motor drivers with staggered pulses to reduce current spikes |
| **TextNavigator** | `TextNavigator.cpp` | Manages the OCR text buffer with current position, next/prev character traversal |
| **ButtonManager** | `ButtonManager.cpp` | Debounced input handling for 4 buttons (Capture, Pause, Prev, Next) |
| **LCDManager** | `LCDManager.cpp` | 16×2 I2C LCD output — displays status messages and current character with its Braille dot pattern |
| **CameraServer** | `camera_server.cpp` | HTTP streaming server on port 81 for live camera preview via web browser |

---

## API Reference

### `POST /scan`

Receives a JPEG image and returns the extracted text.

**Request:**
- Content-Type: `multipart/form-data`
- Body: `file` — a JPEG image file

**Response (200 OK):**
```json
{
  "text": "Hello World"
}
```

**Response (empty OCR result):**
```json
{
  "text": ""
}
```

**Error Responses:**
| Code | Reason |
|---|---|
| `400` | Uploaded file is not an image |
| `500` | Internal server error (Tesseract failure, etc.) |

---

## Troubleshooting

### Backend Issues

| Problem | Solution |
|---|---|
| `tesseract is not installed or it's not in your PATH` | Install Tesseract OCR. On Windows, download from [UB-Mannheim](https://github.com/UB-Mannheim/tesseract/wiki) and install to `C:\Program Files\Tesseract-OCR\`. |
| `uvicorn is not recognized` | Use the virtual environment: `.venv\Scripts\python.exe -m uvicorn backend.main:app --host 0.0.0.0 --port 8000` |
| `ModuleNotFoundError: No module named 'ocr_service'` | Run from the **project root** (not from inside `backend/`), using: `.venv\Scripts\python.exe -m uvicorn backend.main:app ...` |
| `[Errno 10048] address already in use` | Another process is using port 8000. Stop it first, or use a different port: `--port 8001` (and update `API_URL` in Config.h). |

### Firmware Issues

| Problem | Solution |
|---|---|
| `Please specify upload_port` | Set `upload_port = COMx` in `platformio.ini` (check Device Manager for the correct port). |
| Serial Monitor shows nothing | Ensure `monitor_speed = 115200` in `platformio.ini` and `monitor_port` matches your COM port. |
| LCD shows `WiFi Failed` | Double-check `WIFI_SSID` and `WIFI_PASSWORD` in `Config.h`. Ensure your router is on 2.4 GHz (ESP32 does not support 5 GHz). |
| LCD shows `OCR Failed` | The backend server is unreachable. Verify `API_URL` in `Config.h` points to your PC's correct local IP and the server is running. |
| LCD shows `No Text Found` | OCR worked, but no readable text was in the image. Place clear printed text in front of the camera and try again. |
| LCD shows `Cap Error` | Camera failed to capture. Check the FPC cable connection. Try restarting the board. |

### Camera Web Preview

While the system is running, you can view a live camera stream in your browser:

```
http://<ESP32_IP_ADDRESS>:81/stream
```

This is useful for debugging camera angle and focus.

---

## License

This project is developed for academic and assistive technology purposes.
"# AI-Powered-OCR-to-Braille" 
