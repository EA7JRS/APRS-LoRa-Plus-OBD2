/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * APRS LoRa+ OBD-II: pasarela OBD-II → NMEA 2000. Lee el puerto OBD-II del coche por el MCP2515 (CAN a 500 kbit/s, solo
 * consultas) y pone los datos de motor en el bus NMEA 2000 por el TWAI nativo del ESP32-S3 (250 kbit/s, transceptor
 * externo), para que se vean como indicadores de motor en el plotter APRS LoRa+ y en un plotter Garmin.
 * Placa: Seeed XIAO ESP32-S3 + XIAO CAN Bus Expansion Board (MCP2515).
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version. See LICENSE.
 */

#include <Arduino.h>
#include "board.h"
#include "config.h"
#include "obd_link.h"
#include "n2k.h"
#include "net.h"

static String usbLine;

// Instalador web: la placa contesta a "APRS-INFO?" por el USB, como el resto del ecosistema APRS LoRa+
static void usbLoop() {
    while (Serial.available()) {
        const char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (usbLine == "APRS-INFO?")
                Serial.printf("APRS-LoRa-Plus: device=obd2 board=%s version=%s chip=esp32s3 ota=1\n", OBD2_BOARD, OBD2_VERSION);
            usbLine = "";
        } else if (usbLine.length() < 32) usbLine += c;
    }
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.printf("\nAPRS LoRa+ OBD-II %s\n", OBD2_VERSION);
    Serial.println("[INFO][Main] APRS LoRa+ (c) 2026 Manuel - EA7JRS");      // la misma línea en todos los equipos APRS LoRa+
    Config::begin();
    ObdLink::begin();
    N2k::begin();
    Net::begin();
}

void loop() {
    ObdLink::loop();
    N2k::loop(ObdLink::snapshot());
    Net::loop();
    usbLoop();
    delay(1);
}
