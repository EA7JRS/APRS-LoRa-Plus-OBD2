# Hardware — APRS LoRa+ OBD-II

Copyright (C) 2026 Manuel - EA7JRS · GPL-3.0-or-later

Pasarela **OBD-II → NMEA 2000**: lee el motor del coche por el puerto OBD-II y lo pone en un bus NMEA 2000 para que se vea
como indicadores de motor en el plotter APRS LoRa+ y en un plotter Garmin. **Dos buses CAN**, cada uno con su transceptor.

## Material

| Pieza | Para qué |
|---|---|
| **Seeed Studio XIAO ESP32-S3** | WiFi, 8 MB de flash; TWAI (CAN) nativo para el bus NMEA 2000 |
| **XIAO CAN Bus Expansion Board** | MCP2515 (controlador CAN por SPI) + transceptor: es el **bus del coche (OBD-II)**, 500 kbit/s |
| **Transceptor CAN externo** (p. ej. SN65HVD230, 3,3 V) | el **bus NMEA 2000** (250 kbit/s) por el TWAI nativo del ESP32-S3 |
| Convertidor de 12 V a 5 V (mínimo 500 mA) | alimentar la pasarela desde los 12 V del coche |
| Cable OBD-II (conector J1962 de 16 pines) con un extremo libre | llevar el CAN del coche a la borna del kit |
| Conector / derivación NMEA 2000 (Micro-C de 5 pines) | llevar el bus del plotter al transceptor externo |

## Esquema de conexión

```
        COCHE                      PASARELA  (XIAO ESP32-S3)                     PLOTTERS
   ┌──────────────┐        ┌─────────────────────────────────────┐        ┌──────────────────┐
   │ OBD-II J1962 │        │                                     │        │  NMEA 2000 (bus)  │
   │  pin 6  CAN-H├────────┤ MCP2515  (kit CAN)      TWAI nativo │        │                  │
   │  pin 14 CAN-L├────────┤  SPI D8/D9/D10          TX D2 (GPIO3)├───┐    │  APRS LoRa+       │
   │  pin 5  GND  ├──┐     │  CS  D7                 RX D3 (GPIO4)├─┐ │    │  Garmin GPSMAP    │
   │  pin 16 +12V ├─┐│     │                                     │ │ │    └──────────────────┘
   └──────────────┘ ││     └─────────────────────────────────────┘ │ │          ▲   ▲
                    ││              ▲                    │          │ │          │   │
                    ││         500 kbit/s           transceptor    │ └──CAN-L────┤   │
                    ││        (solo lectura)         externo       └────CAN-H────┘   │
                    ││                              (SN65HVD230) ───────GND──────────┘
                    │└── GND ── convertidor 12→5 V ── 5V del XIAO        250 kbit/s
                    └──── +12 V ─┘
```

- **Dos transceptores distintos**: el del kit (MCP2515) para el coche y uno externo para el TWAI nativo. No se pueden
  compartir: son dos buses a velocidades distintas (500 y 250 kbit/s).
- **Masas**: une la masa del coche y la del bus NMEA 2000 **en un solo punto** (la del convertidor). No hagas bucles de masa.

## Pines del XIAO ESP32-S3

| Pin | GPIO | Uso |
|---|---|---|
| D2 | 3 | **NMEA 2000 TX** → transceptor externo (TXD) |
| D3 | 4 | **NMEA 2000 RX** ← transceptor externo (RXD) |
| D7 | 44 | **CS del MCP2515** (kit CAN) |
| D8 / D9 / D10 | 7 / 8 / 9 | **SPI** del MCP2515: SCK / MISO / MOSI |
| 5V / GND | — | alimentación desde el convertidor 12 → 5 V |

Los pines del bus NMEA 2000 (TX/RX) y el CS del MCP2515 se pueden cambiar en la WebUI (*Configuración*) si tu montaje usa otros.

## Bus del coche (OBD-II)

Conector OBD-II (J1962, 16 pines) del coche. Solo hacen falta tres señales para el CAN:

| Pin | Señal | A |
|---|---|---|
| 6 | CAN-H (ISO 15765-4) | **CAN-H** de la borna del kit CAN |
| 14 | CAN-L (ISO 15765-4) | **CAN-L** de la borna del kit CAN |
| 5 | Masa de señal | **GND** de la borna del kit CAN |
| 4 | Masa de chasis | masa del convertidor (opcional) |
| 16 | +12 V permanente | entrada del convertidor de 12 → 5 V (⚠️ está siempre vivo) |

- **500 kbit/s**, CAN de 11 bits (la mayoría de los coches desde 2008; ISO 15765-4). La pasarela **solo consulta** (modo 01):
  manda peticiones al identificador funcional `0x7DF` y escucha las respuestas (`0x7E8`–`0x7EF`). **Nunca** escribe tramas
  propietarias del coche.
- **Terminación**: el bus del coche ya está terminado. **No** cierres el puente de 120 Ω del kit CAN.
- ⚠️ El pin 16 da **12 V aunque el coche esté apagado**: si dejas la pasarela enchufada, consume batería. Usa un cable con
  interruptor o desenchúfala, o aliméntala del encendedor (12 V con la llave).

## Motor de barco (J1939), en lugar del coche

Con *Motor = Barco: J1939* la borna del kit CAN va al bus J1939 del motor (250 kbit/s; conector Deutsch de 9 pines: **C** = CAN-H, **D** = CAN-L, **A** = masa, **B** = +batería). La pasarela solo escucha. El esquema es el mismo cambiando «COCHE» por el motor.

## Bus del barco / plotter (NMEA 2000)

Transceptor externo (SN65HVD230 o equivalente de 3,3 V) entre el TWAI del XIAO y el conector Micro-C:

| Micro-C | Señal | Color habitual | A |
|---|---|---|---|
| 1 | Malla | sin funda | sin conectar (o a GND en un punto) |
| 2 | NET-S (+12 V) | rojo | opcional, para alimentar desde el bus |
| 3 | NET-C (0 V) | negro | **GND** del transceptor y del XIAO |
| 4 | NET-H (CAN H) | blanco | **CANH** del transceptor |
| 5 | NET-L (CAN L) | azul | **CANL** del transceptor |

- **250 kbit/s**, fija. El bus necesita sus **dos terminadores de 120 Ω** en los extremos (como cualquier NMEA 2000).
- El transceptor de 3,3 V (SN65HVD230) va alimentado a **3V3** del XIAO; el SN65HVD230 trae su `Rs` a GND para modo rápido.

## Cristal del MCP2515

Mira lo que pone el cristal plateado del kit CAN (**16.000** u **8.000** MHz). Si no coincide con *MCP2515 → Cristal* de la
WebUI (16 MHz por defecto), el bus del coche no funciona.

> Pendiente de confirmar en la placa: pines exactos del MCP2515 del kit, cristal, y los GPIO del TWAI para el transceptor externo.
