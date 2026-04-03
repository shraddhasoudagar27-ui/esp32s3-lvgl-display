# ESP32-S3 LVGL Display

## Overview
This project interfaces a 1.47-inch SPI LCD display with the ESP32-S3 using ESP-IDF and LVGL.  
It demonstrates low-level display driver integration, GUI rendering, and embedded system design.

## Setup

### Framework
- ESP-IDF v6.1

### Development Environment
- Visual Studio Code
- ESP-IDF VS Code Extension
- Xtensa ESP32 toolchain

### Environment Setup
```bash
C:\esp-idf\export.ps1
```
#### Project Creation
```
idf.py create-project lcd_lvgl
idf.py set-target esp32s3
```
<br>

#### Components / Libraries
LCD: https://thinkrobotics.com/products/1-47inch-lcd-display-module-with-172x320-rounded-corners?variant=44821944238397
ESP32S3: https://www.amazon.in/SparkFun-WRL-24408-Thing-Plus-ESP32-S3/dp/B0D1BB79SM

#### LVGL (GUI Library)
<br> 
Added manually:
<br> 
Used for:
UI rendering
Widgets and layouts
ESP-IDF Built-in Components
esp_lcd → LCD panel driver (SPI + ST7789)
FreeRTOS → Task scheduling
<br> 

#### Display Configuration
Display Controller: ST7789
Interface: SPI (SPI2_HOST)
Color Format: RGB565 (16-bit)

Driver initialized using:
```
esp_lcd_new_panel_st7789()
```
<br>

#### Hardware Connections
| LCD Pin | ESP32-S3 GPIO |
|--------|--------------|
| VCC    | 3.3V         |
| GND    | GND          |
| DIN    | GPIO11       |
| CLK    | GPIO12       |
| CS     | GPIO10       |
| DC     | GPIO9        |
| RST    | GPIO8        |
| BL     | GPIO7        |

#### Configuration (menuconfig)

Configured via:
```
idf.py menuconfig
```
LVGL
Enabled LVGL
OS: FreeRTOS
Demo widgets enabled
Fonts
```
Montserrat 14
Montserrat 20
Montserrat 28
Montserrat 48
```

#### Build & Flash
```
idf.py build
idf.py -p <PORT> flash monitor
```
#### Features
LVGL-based GUI rendering
SPI-driven LCD communication
Custom embedded display pipeline using esp_lcd
Real-time UI updates on ESP32-S3

##### Notes:
- No Arduino / PlatformIO used
- Pure ESP-IDF implementation
- LVGL integrated manually (not via package manager)
