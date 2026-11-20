# APRS LoRa+ OBD-II — contexto del proyecto

Propietario: Manuel (EA7JRS). Idioma de trabajo: español, respuestas breves y directas.
Archivos nuevos con la cabecera `Copyright (C) 2026 Manuel - EA7JRS` y licencia GPL-3.0-or-later.
Repositorio: `EA7JRS/APRS-LoRa-Plus-OBD2` (por crear). Parte del ecosistema APRS LoRa+ (tracker, rotor, app, display, plotter, pasarela NMEA2000, sat-aprs).

## Qué es
Uso del «kit CAN» de Manuel (XIAO ESP32-S3 + XIAO CAN Bus Expansion Board): pasarela **OBD-II → NMEA 2000**. Lee el OBD-II del coche y emite los datos de motor al bus NMEA 2000 para que se vean como indicadores de motor en el **plotter APRS LoRa+** y en un **plotter Garmin**. Opcional: telemetría APRS al tracker/app.

## Decisiones (Manuel, 2026-11-14)
- Objetivo: que los indicadores de motor funcionen **tanto en el plotter APRS LoRa+ como en el Garmin** → emitir **PGN de motor estándar** (127488/127489/127505/127508); ambos plotters los leen sin código específico.
- **Doble CAN**: OBD-II (coche, 500 kbit/s) por el **MCP2515** del kit; NMEA 2000 (250 kbit/s) por el **TWAI nativo** del ESP32-S3 con un **transceptor externo**. Dos buses = dos transceptores.
- **Solo lectura** del coche: consultas OBD-II estándar (modo 01), nunca se escribe en el bus del coche.

## Estructura
- `lib/ObdCore/src/`: núcleo C++ sin Arduino.
  - `obd.h`: OBD-II ISO 15765-4 / SAE J1979 (petición del modo 01, decodificación de PID a magnitudes físicas; lista de PID a consultar por turnos).
  - `n2k_engine.h`: identificador de 29 bits, NAME, reclamo de dirección ISO (60928/59904) y codificadores de los PGN de motor (127488 rápido, 127489 dinámico fast packet, 127505 nivel de combustible, 127508 batería) + partido de fast packet. Formatos contrastados con la librería NMEA2000 de ttlappalainen.
- `test_host/`: pruebas en el PC (`make run`; 20 comprobaciones). PID decodificados contra ejemplos J1979; PGN contra los valores de la librería NMEA2000.
- `src/`: firmware (por hacer): lectura OBD-II por el MCP2515, emisión NMEA 2000 por el TWAI, WebUI, config, telemetría APRS.
- Entorno: `xiao_esp32s3_obd2` (espressif32 8.0.0, `seeed_xiao_esp32s3`, `default_8MB.csv`). Librerías: CAN_BUS_Shield (MCP2515), NimBLE 2.5.2, ArduinoJson 7.4.3.

## PID → PGN (mapa)
- RPM (0x0C) → 127488 EngineSpeed; MAP (0x0B) → 127488 presión de turbo.
- Refrigerante (0x05) → 127489 CoolantTemp; aceite (0x5C) → OilTemp; consumo (0x5E) → FuelRate; tensión del módulo (0x42) → AltVoltage y 127508; horas/marcha (0x1F) → EngineHours; carga (0x04) → Load.
- Nivel de combustible (0x2F) → 127505 (tipo 0 = combustible).

## Estado (2026-11-14)
- Núcleo probado en el PC (20 comprobaciones). Firmware, WebUI y release: **en preparación**. Sin probar con un coche, un transceptor, ni un plotter.
- Falta: drivers de las dos CAN (MCP2515 + TWAI), bucle de consulta OBD-II, emisión de los PGN, WebUI, config, CI, release en el instalador web (grupo propio o «kit CAN»), y la telemetría APRS opcional.

## Versión
- **5.4.7** (2026-11-20; Manuel: «subimos todo a la siguiente version»): `OBD2_VERSION` en `platformio.ini` (lo contesta `APRS-INFO?`), README. Mismo número que el resto del ecosistema.

## Cómo trabajar
- No publicar releases ni crear repositorios sin que lo pida Manuel.
- Antes de subir: `make -C test_host run`; la CI compilará el firmware.
