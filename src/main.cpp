/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// Firmware (esqueleto): por hacer la lectura OBD-II por el MCP2515 y la emisión NMEA 2000 por el TWAI. De momento solo
// responde a "APRS-INFO?" por el puerto serie, como el resto de equipos del ecosistema.
#include <Arduino.h>
#include "obd.h"
#include "n2k_engine.h"

#ifndef OBD2_VERSION
#define OBD2_VERSION "5.4.7"   // platformio.ini lo define; el mismo numero que el resto del ecosistema APRS LoRa+
#endif

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("APRS LoRa+ OBD-II (esqueleto)");
}

void loop() {
    if (Serial.available()) {
        String s = Serial.readStringUntil('\n');
        s.trim();
        if (s == "APRS-INFO?") {
            Serial.printf("APRS-LoRa-Plus: device=obd2 board=xiao_esp32s3 version=%s chip=%s\n", OBD2_VERSION, ESP.getChipModel());
        }
    }
    delay(10);
}
