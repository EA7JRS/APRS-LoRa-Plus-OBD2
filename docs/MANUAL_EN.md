# APRS LoRa+ OBD-II — User manual

Copyright (C) 2026 Manuel - EA7JRS · GPL-3.0-or-later · version 5.4.7 · part of the **APRS LoRa+** ecosystem.

## 1. What it is

An **OBD-II → NMEA 2000** gateway. It reads the car's engine through the diagnostic port (OBD-II) and publishes the data on an
NMEA 2000 bus, so it shows up as **engine instruments** on both the **APRS LoRa+ plotter** and a **Garmin plotter** (or any
NMEA 2000 device). Because it uses the standard engine PGNs, no plotter needs special code.

The board is a **Seeed XIAO ESP32-S3** with the **CAN kit** (MCP2515) for the car and an **external CAN transceiver** for the
NMEA 2000 bus. They are two separate buses, at 500 and 250 kbit/s. See [HARDWARE.md](HARDWARE.md).

## 2. What it reads from the car

Standard OBD-II requests (ISO 15765-4 / SAE J1979, **mode 01**, read only). It polls these PIDs in turn; any the ECU does not
answer show as "no data":

| Value | PID | Unit |
|---|---|---|
| Engine speed | 0x0C | rpm |
| Speed | 0x0D | km/h |
| Coolant temperature | 0x05 | °C |
| Oil temperature | 0x5C | °C |
| Intake temperature | 0x0F | °C |
| Engine load | 0x04 | % |
| Fuel level | 0x2F | % |
| Instant fuel rate | 0x5E | L/h |
| Control module voltage | 0x42 | V |
| Manifold pressure (MAP) | 0x0B | kPa |
| Run time | 0x1F | s |

Not every car provides every PID; the gateway emits only what it receives.

## 3. What it sends to the NMEA 2000 bus

| PGN | Carries | Every |
|---|---|---|
| **127488** Engine Parameters, Rapid | engine speed and boost pressure | 100 ms |
| **127489** Engine Parameters, Dynamic | coolant, oil, fuel rate, alternator voltage, hours, load | 500 ms |
| **127505** Fluid Level | fuel level (type 0) and tank capacity | 2 s |
| **127508** Battery Status | control module voltage as battery voltage | 1 s |

Mapping: coolant and oil go to kelvin; module voltage goes out as *AltVoltage* (127489) and as battery voltage (127508); if
*MAP as boost* is on, 127488 carries the gauge pressure (MAP − 101.325 kPa, floor 0). Tank capacity is sent only if you set it
in *Configuration*.

The gateway **claims an address** on the bus (ISO 11783-5) and defends it; the resulting address is stored and reused on
start-up. It is not a certified node.

## 4. WebUI

Join the **APRS-OBD2** WiFi access point (password `aprsobd2`) and open `http://192.168.4.1`. Three panels:

- **Engine (OBD-II)**: engine speed, speed, temperatures, load, fuel, fuel rate, voltage, manifold pressure and run time, as
  answered by the ECU.
- **OBD-II bus**: whether the MCP2515 started, whether the ECU responds, and the request / response / error counters.
- **NMEA 2000**: whether the driver is running, the claimed address (or "lost"), whether it is claimed, and the frames sent and
  received.

To **save** the configuration or **update** the firmware you need the key (user `admin`, password `123456` by default; it is
the APRS LoRa+ ecosystem PIN, changed or left empty in *Configuration*).

## 5. Configuration

| Setting | What it is |
|---|---|
| Name / access point | name of the access point, mDNS and network |
| WebUI password | PIN to save and update (empty = no key) |
| WiFi | access point, client (with rescue access point) or off |
| MCP2515 CS, crystal | chip-select pin (D7 = 44) and kit crystal (16 or 8 MHz) |
| OBD-II poll (ms) | time between requests (200 by default) |
| Data timeout (ms) | if a value does not arrive within this time it is marked "no data" (1500 by default) |
| NMEA 2000 TX / RX | external transceiver pins (D2 = 3 / D3 = 4) |
| NMEA 2000 address | preferred bus address (negotiated) |
| Engine instance | engine number in the PGNs (0 = port / single) |
| MAP as boost | send MAP as gauge boost pressure in 127488 |
| Tank capacity (L) | for 127505 (0 = no data) |

## 6. Safety

- **Read only**: the gateway only sends standard OBD-II requests (mode 01) to identifier `0x7DF`. It **never** writes frames to
  the car's bus.
- OBD-II pin 16 is live 12 V even with the car off: do not leave the gateway plugged in or it will drain the battery.
- Bond the car and boat grounds at a single point only (the converter's).

## 7. Troubleshooting

- **ECU not responding**: check the MCP2515 crystal (16 or 8 MHz), that the ignition is on, and the CAN-H / CAN-L wiring. Some
  cars need the engine running for certain PIDs.
- **Plotter shows no engine**: check *NMEA 2000 → Claimed = yes* and that the bus has its two 120 Ω terminators. On the plotter,
  enable the engine page or instruments and, if it asks for an instance, use 0.
- **No NMEA 2000 bus**: check the external transceiver (powered at 3V3, CANH/CANL to the bus) and the TX/RX pins.

## 8. Updating the firmware

Through the WebUI, in *Update firmware* (`/update`) with the key; over USB with the [web installer](https://ea7jrs.github.io/APRS-LoRa-Plus/install/#nautica) (**Nautical** group, board `xiao_esp32s3_obd2`; settings are kept), or building it with `pio run -e xiao_esp32s3_obd2 -t upload`.
Over USB the board answers `APRS-INFO?` with `device=obd2`.

## 9. Boat engine (J1939)

In *Configuration > Motor* choose **Barco: J1939** for a marine or industrial diesel with SAE J1939 (29-bit CAN, 250 kbit/s; some at 500). In that mode the gateway **only listens**: it sends no requests. Wire CAN-H / CAN-L / GND of the kit terminal to the engine's J1939 bus (9-pin Deutsch diagnostic connector: C = CAN-H, D = CAN-L, A = ground; or as the maker states). Do not close the 120 Ω jumper if the bus already has its terminators.

| Value | J1939 PGN | Goes out in |
|---|---|---|
| Engine speed | 61444 (EEC1) | 127488 |
| Boost, intake temperature | 65270 | 127488 |
| Load | 61443 (EEC2) | 127489 |
| Coolant, oil (temperature) | 65262 | 127489 |
| Oil, coolant, fuel (pressure) | 65263 | 127489 |
| Fuel rate | 65266 | 127489 |
| Total hours | 65253 | 127489 |
| Battery voltage | 65271 | 127489 and 127508 |
| Fuel level | 65276 | 127505 |

Many engines give total hours only on request; as the gateway requests nothing, they appear only if the engine broadcasts them. Not tested with a real engine.

## 10. APRS telemetry (pending)

Sending engine telemetry over APRS to the tracker and the app is planned; for now only the preference is stored.

## 11. Formats

PGN formats (NMEA 2000 and J1939) follow the [CANboat](https://canboat.github.io/canboat/) database v9.2.0 (Apache-2.0); `tools/canboat_run.sh` checks them.
