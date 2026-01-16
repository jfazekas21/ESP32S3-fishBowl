# ESP32S3-fishBowl

## Configuration

User-facing firmware configuration lives in `CONFIG.md`.

## ESP-IDF recommendation

For production builds, use the latest stable ESP-IDF release. As of Jan 16, 2026, the recommended stable version is `v5.5.2`, and we will keep using this version until it is manually updated. Avoid `master` or pre-release branches for production.

## Hardware

Hardware artifacts live in `Hardware/`:
- Schematic: `Hardware/B541 RevC3 (SmartPOE Controller).PDF`
- BOM: `Hardware/B541 RevC3 (SmartPOE Controller).csv` (also `.xlsx`)

## CPU layout (from BOM + schematic)

The main processor is the `ESP32-S3-WROOM-1-N16R8` module (designator `IC1`). It is the central block in the schematic and is surrounded by:
- `J2` USB-C connector for USB/power.
- `J1` board connector and `CB1` 10-pin 2x5 SWD cable for debug access.
- `P1`/`P2` 2x16-pin headers that break out module signals.
- `IC2` RTC with coin cell `BT1` for timekeeping.

See the schematic PDF in `Hardware/` for the detailed pin map and net names.

## User feedback I/O

- RGB status LED (`D1`) uses three GPIOs:
  - Red: `IO1`
  - Green: `IO2`
  - Blue: `IO3`
- Supply indicator LED (`D2`) is driven through `R7` from `3V3`.
- Pushbuttons:
  - `SW1` is the reset button, tied to `RST` with a pull-up (`R1`) and cap (`C1`).
  - `SW2` is a user button on `IO38` with a pull-up (`R17`) to `3V3`.

## SD card and USB mapping

- SD card (`J1`) signal mapping:
  - `IO46` → `CS`
  - `IO40` → `MOSI`
  - `IO41` → `SCK`
  - `IO42` → `MISO`
  - `IO45` → `DET`
- USB:
  - `IO19` → `USB+`
  - `IO20` → `USB-`
  - USB signals are routed through `U1` (`USBLC6-2SC6`) for protection.
- UART0:
  - `IO43` → `TXD0`
  - `IO44` → `RXD0`