/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
#pragma once
#include <Arduino.h>
#include "obd.h"

// Lee el OBD-II del coche por el MCP2515 (CAN de 11 bits a 500 kbit/s): consulta los PID del modo 01 por turnos y decodifica
// las respuestas de la ECU. Solo consulta, nunca escribe en el bus propietario del coche.
namespace ObdLink {
    void        begin();
    void        loop();
    Obd::Data   snapshot();          // los campos sin respuesta reciente (timeoutMs) salen a NAN
    bool        busOk();             // el MCP2515 arrancó
    bool        ecuAlive();          // la ECU ha respondido hace poco
    uint32_t    requests();
    uint32_t    responses();
    uint32_t    rxErrors();
}
