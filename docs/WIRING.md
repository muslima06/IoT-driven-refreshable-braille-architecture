# Wiring Guide

## ESP32-S3-N16R8 (Freenove FNK0086) to Components

> **IMPORTANT:** GPIOs 4–13, 15–18 are hardwired to the on-board OV3660 camera
> connector and MUST NOT be used for any other peripheral.
> GPIOs 26–37 are reserved for PSRAM/Flash on the N16R8 module.

### OV3660 Camera (Hardwired — No User Wiring Required)
The camera is connected via the board's FPC connector. These pins are fixed:
* XCLK  -> GPIO15
* SIOD  -> GPIO4  (I2C SDA to camera)
* SIOC  -> GPIO5  (I2C SCL to camera)
* Y2    -> GPIO11
* Y3    -> GPIO9
* Y4    -> GPIO8
* Y5    -> GPIO10
* Y6    -> GPIO12
* Y7    -> GPIO18
* Y8    -> GPIO17
* Y9    -> GPIO16
* VSYNC -> GPIO6
* HREF  -> GPIO7
* PCLK  -> GPIO13

### I2C LCD (16x2)
* SDA -> GPIO21
* SCL -> GPIO14
* VCC -> 5V (use level shifter if LCD is strictly 5V)
* GND -> GND

### DRV8833 Motor Drivers & Solenoids
* **DRV8833 #1** (Solenoids 1 & 2)
  * AIN1 -> GPIO38
  * AIN2 -> GPIO39
  * BIN1 -> GPIO40
  * BIN2 -> GPIO41
  * STBY -> Wire to 3.3V (Hardware enable)
* **DRV8833 #2** (Solenoids 3 & 4)
  * AIN1 -> GPIO1
  * AIN2 -> GPIO2
  * BIN1 -> GPIO3
  * BIN2 -> GPIO42
  * STBY -> Wire to 3.3V (Hardware enable)
* **DRV8833 #3** (Solenoids 5 & 6)
  * AIN1 -> GPIO45
  * AIN2 -> GPIO46
  * BIN1 -> GPIO47
  * BIN2 -> GPIO48
  * STBY -> Wire to 3.3V (Hardware enable)

> **Note on Serial Monitor:** We have carefully avoided GPIO43 (TX). This means the Serial Monitor output will still work perfectly for reading your IP address!

### Push Buttons
* Capture Button  -> **GPIO 0** (Use the on-board BOOT button, or wire a button to GPIO0)
* Pause Button    -> GPIO19 (to GND, uses INPUT_PULLUP)
* Previous Button -> GPIO20 (to GND, uses INPUT_PULLUP)
* Next Button     -> GPIO44 (to GND, uses INPUT_PULLUP. This is the RX pin, but perfectly safe as a button input!)

### Power Supply
* External 5V 5A power supply should be connected to the ESP32 VBUS/5V pin and shared with the motor drivers and LCD. Connect all grounds together.

### Pin Allocation Summary

| GPIO | Function           | Notes                    |
|------|--------------------|--------------------------|
| 1    | DRV2 AIN1          | Solenoid 3 forward       |
| 2    | DRV2 AIN2          | Solenoid 3 reverse       |
| 3    | DRV2 BIN1          | Solenoid 4 forward       |
| 4    | Camera SIOD        | **Hardwired — do not use** |
| 5    | Camera SIOC        | **Hardwired — do not use** |
| 6    | Camera VSYNC       | **Hardwired — do not use** |
| 7    | Camera HREF        | **Hardwired — do not use** |
| 8    | Camera Y4          | **Hardwired — do not use** |
| 9    | Camera Y3          | **Hardwired — do not use** |
| 10   | Camera Y5          | **Hardwired — do not use** |
| 11   | Camera Y2          | **Hardwired — do not use** |
| 12   | Camera Y6          | **Hardwired — do not use** |
| 13   | Camera PCLK        | **Hardwired — do not use** |
| 14   | I2C SCL (LCD)      |                          |
| 15   | Camera XCLK        | **Hardwired — do not use** |
| 16   | Camera Y9          | **Hardwired — do not use** |
| 17   | Camera Y8          | **Hardwired — do not use** |
| 18   | Camera Y7          | **Hardwired — do not use** |
| 19   | DRV2 BIN2          | Solenoid 4 reverse       |
| 20   | DRV3 AIN1          | Solenoid 5 forward       |
| 21   | I2C SDA (LCD)      |                          |
| 38   | DRV1 AIN1          | Solenoid 1 forward       |
| 39   | DRV1 AIN2          | Solenoid 1 reverse       |
| 40   | DRV1 BIN1          | Solenoid 2 forward       |
| 41   | DRV1 BIN2          | Solenoid 2 reverse       |
| 42   | Capture Button     | INPUT_PULLUP             |
| 43   | DRV3 BIN1          | Solenoid 6 fwd (TX pin)  |
| 44   | DRV3 BIN2          | Solenoid 6 rev (RX pin)  |
| 45   | Pause Button       | INPUT_PULLUP             |
| 46   | DRV3 AIN2          | Solenoid 5 reverse       |
| 47   | Previous Button    | INPUT_PULLUP             |
| 48   | Next Button        | INPUT_PULLUP             |
