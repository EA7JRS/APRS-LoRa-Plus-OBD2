# APRS LoRa+ OBD-II

Copyright (C) 2026 Manuel - EA7JRS · GPL-3.0-or-later · versión **5.5.0** (la misma que el resto del ecosistema) · parte del ecosistema **APRS LoRa+**.

Instalación sin compilar: [instalador web de APRS LoRa+](https://ea7jrs.github.io/APRS-LoRa-Plus/install/#nautica), grupo **Náutica** (Chrome o Edge, cable USB).

Pasarela **OBD-II → NMEA 2000**: lee el puerto OBD-II del coche y pone los datos de motor en el bus **NMEA 2000**, para que se vean como **indicadores de motor** tanto en el **plotter APRS LoRa+** como en un **plotter Garmin** (y cualquier equipo NMEA 2000). También puede mandar la telemetría por **APRS** al tracker y la app (pendiente).

```
 Coche (OBD-II, CAN 500 kbit/s) ──MCP2515──► XIAO ESP32-S3 ──TWAI + transceptor──► NMEA 2000 (250 kbit/s) ──► plotters
```

- **Lee** por OBD-II (ISO 15765-4 / SAE J1979, modo 01, **solo consultas**, nunca escribe en el bus del coche): RPM, velocidad, temperatura del refrigerante y del aceite, carga, nivel y consumo de combustible, tensión del módulo, horas de marcha, presión del colector...
- **Emite** al bus NMEA 2000 los **PGN de motor estándar**: **127488** (rápido: RPM, turbo), **127489** (dinámico: temperaturas, consumo, tensión, horas, carga), **127505** (nivel de combustible) y **127508** (batería). Al ser estándar, los dos plotters los pintan sin código específico.
- Reclama una dirección en el bus NMEA 2000 (ISO 11783-5). No es un nodo certificado.

## Motor de barco (J1939)

Además del OBD-II del coche, la pasarela puede escuchar un **motor diésel marino o industrial con SAE J1939** (*Configuración > Motor > Barco: J1939*): el MCP2515 pasa a 250 kbit/s (o 500) y **solo escucha** lo que el motor difunde (régimen, carga, temperaturas, presiones de aceite / refrigerante / combustible, consumo, sobrealimentación, tensión, nivel de combustible y horas totales), y lo emite con los mismos PGN NMEA 2000. No pide ni escribe nada en el bus del motor.

## Referencia de formatos: CANboat

Los formatos de los PGN (NMEA 2000 y J1939) son los de la base de datos de **[CANboat](https://canboat.github.io/canboat/)** (Apache-2.0), release fijada **v9.2.0**. `tools/canboat_run.sh` baja `canboat.json` y `canboat-j1939.json` de esa release y comprueba con ellos nuestros codificadores y el decodificador J1939 (59 comprobaciones); la CI lo ejecuta en cada subida.

## Hardware

- **XIAO ESP32-S3** + **XIAO CAN Bus Expansion Board** (MCP2515 + transceptor): el MCP2515 se conecta al **OBD-II del coche** (500 kbit/s) con un cable OBD-II a DB9/CAN-H/CAN-L.
- Para el **segundo bus (NMEA 2000, 250 kbit/s)** hace falta un **transceptor CAN externo** (p. ej. SN65HVD230) en el **TWAI nativo** del ESP32-S3 (dos GPIO libres). Dos buses = dos transceptores. Detalle en [`docs/HARDWARE.md`](docs/HARDWARE.md).

## Estado

- Núcleo probado en el PC (`test_host`, 28 comprobaciones): decodificador OBD-II y codificadores de PGN de motor contrastados con canboat.
- **Firmware (2026-11-15):** lectura OBD-II por el MCP2515 (`obd_link`, consulta de PID por turnos, caducidad por dato), emisión NMEA 2000 por el TWAI nativo con reclamo de dirección (`n2k`, 127488/127489/127505/127508), configuración en LittleFS (`config`) y WebUI con la cabecera del ecosistema y botón de donativo (`net`, `web_page.h`): estado del motor y del bus, ajustes y actualización por `/update`. Responde a `APRS-INFO?` por USB (`device=obd2`). **Compila en la CI. Sin probar con un coche, un transceptor NMEA 2000 ni un plotter.**
- Falta: telemetría APRS opcional al tracker/app, comprobar pines, cristal del MCP2515 y los PGN con un vehículo y un plotter reales.

## Compilar

```
make -C test_host run            # pruebas del núcleo en el PC
tools/canboat_run.sh             # formatos contra CANboat v9.2.0
pio run -e xiao_esp32s3_obd2     # firmware (PlatformIO)
```

## Estructura

```
lib/ObdCore/src/   núcleo en C++ sin Arduino: obd.h (OBD-II J1979) y n2k_engine.h (PGN de motor, reclamo de dirección)
src/               firmware: board, config, obd_link (MCP2515), n2k (TWAI), net + WebUI, main
test_host/         pruebas en el PC
```

## Documentación

- [docs/HARDWARE.md](docs/HARDWARE.md) — material, pines, conector OBD-II, bus NMEA 2000 y esquema de conexión.
- Guía rápida: [docs/QUICKSTART_ES.md](docs/QUICKSTART_ES.md) · [EN](docs/QUICKSTART_EN.md).
- Manual de usuario: [docs/MANUAL_ES.md](docs/MANUAL_ES.md) · [EN](docs/MANUAL_EN.md).

Librerías: CAN_BUS_Shield (Seeed/ttlappalainen, MIT) para el MCP2515 y ArduinoJson. El bus NMEA 2000 va por el TWAI nativo del ESP32-S3 sin librería externa (`lib/ObdCore/src/n2k_engine.h`).
