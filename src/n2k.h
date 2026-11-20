/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
#pragma once
#include <Arduino.h>
#include "obd.h"

// Emite los datos de motor al bus NMEA 2000 por el TWAI nativo del ESP32-S3 (250 kbit/s, transceptor externo): reclama una
// dirección (ISO 11783-5) y manda 127488, 127489, 127505 y 127508 desde lo que se lee del OBD-II.
namespace N2k {
    struct Status {
        bool     running = false;
        bool     claimed = false;
        bool     lost = false;
        uint8_t  address = 254;
        uint32_t sent = 0;
        uint32_t received = 0;
        uint32_t claims = 0;
    };
    void          begin();
    void          loop(const Obd::Data& d);
    const Status& status();
}
