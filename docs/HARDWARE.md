# Hardware — APRS LoRa+ OBD-II

Copyright (C) 2026 Manuel - EA7JRS · GPL-3.0-or-later

## Placas

- **Seeed XIAO ESP32-S3**.
- **XIAO CAN Bus Expansion Board** (MCP2515 + transceptor TJA1050/SN65HVD): el **MCP2515** es el **bus del coche (OBD-II)**.
- **Transceptor CAN externo** (p. ej. SN65HVD230, 3,3 V) para el **TWAI nativo** del ESP32-S3: el **bus NMEA 2000**.

## Dos buses CAN

| Bus | Velocidad | Controlador | Transceptor | Conector |
|---|---|---|---|---|
| OBD-II (coche) | **500 kbit/s** | MCP2515 (kit CAN) | el del kit | cable OBD-II (DB9 → OBD-II): CAN-H pin 6, CAN-L pin 14, GND pin 4/5; **no** se usa la alimentación del coche para el XIAO salvo con un convertidor a 5 V |
| NMEA 2000 | **250 kbit/s** | TWAI nativo (ESP32-S3) | externo (SN65HVD230) | derivación NMEA 2000 (CAN-H, CAN-L, GND) |

- El MCP2515: CS, SPI (SCK/MISO/MOSI) y la interrupción del kit (ver el esquema del XIAO CAN Bus Expansion Board). Cristal del MCP2515: **comprobar en la placa** (8 o 16 MHz).
- El TWAI nativo: dos GPIO libres del XIAO (TX/RX) al transceptor externo; configurado a 250 kbit/s.
- **Solo lectura del coche**: la pasarela solo envía consultas OBD-II estándar (modo 01) a la ECU; nunca escribe tramas del bus propietario del coche.
- Avería eléctrica: aislar bien los dos buses; no unir masas del coche y del barco si no comparten referencia.

> Pendiente de confirmar en la placa: pines exactos del MCP2515 del kit, cristal, y los GPIO del TWAI para el transceptor externo.
