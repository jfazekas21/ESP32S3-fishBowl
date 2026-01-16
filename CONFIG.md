# User-Facing Configuration

This document captures configuration values that affect user-facing behavior.
Update these values to match the current firmware build settings.

## Serial interfaces

### UART0 (boot + early logs)

- Pins:
  - `IO43` → `TXD0`
  - `IO44` → `RXD0`
- Use UART0 for ROM boot logs and early startup messages.
- Default baud rate: **921600**
 - Developer debug logs: route `ESP_LOG*` output to UART0.

### USB CDC (runtime logs)

- Pins:
  - `IO19` → `USB+`
  - `IO20` → `USB-`
- Recommended for higher-throughput runtime logging.
- Default CDC line coding / baud rate: **460800**
 - User console: use USB CDC for interactive/user-facing console I/O.

## Partition layout

> Fill these sizes to match your build configuration (e.g., `partitions.csv`).

- Secondary bootloader size: **TBD**
- Application slot size: **TBD**
