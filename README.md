# ESP32S3-fishBowl

Base project for ESP32S3 using the latest ESP-IDF framework (v5.3.2).

## Requirements

- ESP-IDF v5.3 or later
- ESP32S3 development board

## Setup

1. Install ESP-IDF by following the [official guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html)

2. Set up the ESP-IDF environment:
   ```bash
   . $HOME/esp-idf/export.sh
   ```

## Build

To build the project:

```bash
idf.py set-target esp32s3
idf.py build
```

## Flash

To flash the project to your ESP32S3:

```bash
idf.py -p PORT flash
```

Replace `PORT` with your serial port (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).

## Monitor

To monitor the serial output:

```bash
idf.py -p PORT monitor
```

## Build, Flash and Monitor

You can combine all steps:

```bash
idf.py -p PORT flash monitor
```

## Project Structure

```
.
├── CMakeLists.txt          # Project CMake file
├── main/
│   ├── CMakeLists.txt      # Main component CMake file
│   └── main.c              # Main application code
├── sdkconfig.defaults      # Default SDK configuration for ESP32S3
└── README.md               # This file
```

## ESP-IDF Version

This project is built with ESP-IDF v5.3.2.
