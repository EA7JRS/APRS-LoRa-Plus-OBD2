# APRS LoRa+ OBD-II — contexto del proyecto

Propietario: Manuel (EA7JRS). Idioma de trabajo: español, respuestas breves y directas.
Archivos nuevos con la cabecera `Copyright (C) 2026 Manuel - EA7JRS` y licencia GPL-3.0-or-later.
Repositorio: `EA7JRS/APRS-LoRa-Plus-OBD2` (público, creado por Manuel). Parte del ecosistema APRS LoRa+ (tracker, rotor, app, display, plotter, pasarela NMEA2000, sat-aprs).

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
- Entorno: `xiao_esp32s3_obd2` (espressif32 8.0.0, `seeed_xiao_esp32s3`, `default_8MB.csv`). Librerías: CAN_BUS_Shield (MCP2515), ArduinoJson 7.4.3. NMEA 2000 por el TWAI nativo (sin librería; `n2k_engine.h` propio). WebUI, OTA, LittleFS y mDNS del core de Arduino.

## PID → PGN (mapa)
- RPM (0x0C) → 127488 EngineSpeed; MAP (0x0B) → 127488 presión de turbo.
- Refrigerante (0x05) → 127489 CoolantTemp; aceite (0x5C) → OilTemp; consumo (0x5E) → FuelRate; tensión del módulo (0x42) → AltVoltage y 127508; horas/marcha (0x1F) → EngineHours; carga (0x04) → Load.
- Nivel de combustible (0x2F) → 127505 (tipo 0 = combustible).

## Estado (2026-11-15)
- Núcleo probado en el PC (20 comprobaciones).
- **Firmware (2026-11-15):** `src/board.h` (pines XIAO: MCP2515 CS D7=GPIO44 por SPI D8/D9/D10; TWAI TX D2=GPIO3 / RX D3=GPIO4; `OBD2_VERSION` 0.1.0), `src/config.*` (LittleFS `/obd_conf.json`: WiFi, OBD cs/mhz/pollMs/timeoutMs, NMEA 2000 tx/rx/addr/instancia, `mapAsBoost`, `fuelCapacityL`, `aprsOut` pendiente), `src/obd_link.*` (MCP2515 con CAN_BUS_Shield a 500 kbit/s; consulta los PID de `pollList` por turnos; `snapshot()` pone a NAN lo que no llega en `timeoutMs`; solo lee), `src/n2k.*` (TWAI nativo a 250 kbit/s con `N2kEng::AddressClaim`; emite 127488 cada 100 ms, 127489 fast packet cada 500 ms, 127505 cada 2 s y 127508 cada 1 s cuando hay dato; guarda la dirección negociada en NVS `obd2n2k/addr`), `src/net.*` + `src/web_page.h` (WiFi AP/cliente, WebUI con la cabecera del ecosistema y donativo PayPal.me, `/api/status`, `/api/config`, `/update`), `src/main.cpp` (`APRS-INFO?` → `device=obd2`). Quitado NimBLE de `platformio.ini` (no se usa). **Compila en la CI. Sin probar con un coche, un transceptor ni un plotter.**
- MAP → 127488 como sobrealimentación manométrica (MAP − 101,325 kPa, mínimo 0) si `mapAsBoost`; refrigerante/aceite a kelvin; tensión del módulo a `AltVoltage` (127489) y 127508; nivel 0x2F → 127505 tipo 0.
- Falta: telemetría APRS opcional al tracker/app; release en el instalador web (grupo «kit CAN» / Náutica); comprobar en la placa los pines, el cristal del MCP2515 y los PGN con un vehículo y un plotter reales.

## Cómo trabajar
- No publicar releases ni crear repositorios sin que lo pida Manuel.
- Antes de subir: `make -C test_host run`; la CI compilará el firmware.
