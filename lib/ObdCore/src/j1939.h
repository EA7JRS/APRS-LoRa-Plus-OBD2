/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// SAE J1939 (motores diésel marinos e industriales, CAN de 29 bits, normalmente a 250 kbit/s): decodifica los PGN de motor
// que el motor difunde por su cuenta. Solo escucha: no se pide ni se escribe nada. Deja los datos en la misma estructura
// que el OBD-II (Obd::Data). Formatos contrastados con la base de datos J1939 de CANboat (canboat-j1939.json, v9.2.0;
// tools/canboat_check.py lo repite en la CI). Sin Arduino.
#pragma once
#include <stdint.h>
#include "obd.h"

namespace J1939 {

enum Pgn : uint32_t {
    PGN_EEC2 = 61443,      // carga del motor
    PGN_EEC1 = 61444,      // régimen
    PGN_HOURS = 65253,     // horas totales (bajo petición en muchos motores; algunos lo difunden)
    PGN_ET1 = 65262,       // temperaturas: refrigerante, aceite
    PGN_EFLP1 = 65263,     // presiones: combustible, aceite, refrigerante
    PGN_LFE = 65266,       // consumo
    PGN_IC1 = 65270,       // sobrealimentación, temperatura de admisión
    PGN_VEP1 = 65271,      // tensión de batería
    PGN_DD = 65276,        // nivel de combustible
};

// PGN de un identificador de 29 bits (PDU1: el byte de destino no forma parte del PGN)
inline uint32_t pgnOf(uint32_t id) {
    const uint32_t pf = (id >> 16) & 0xFF, dp = (id >> 24) & 3;
    return pf < 240 ? (dp << 16) | (pf << 8) : (dp << 16) | (pf << 8) | ((id >> 8) & 0xFF);
}
inline uint8_t sourceOf(uint32_t id) { return (uint8_t)(id & 0xFF); }

inline bool ok8(uint8_t v) { return v <= 0xFA; }            // 0xFB..0xFF: fuera de rango, error o sin dato
inline bool ok16(uint16_t v) { return v <= 0xFAFF; }
inline uint16_t le16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }

// Decodifica una trama. Devuelve el PGN si traía algún dato que usamos, o 0.
inline uint32_t decode(uint32_t id, const uint8_t* d, int len, Obd::Data& out) {
    if (len < 8) return 0;
    const uint32_t pgn = pgnOf(id);
    bool any = false;
    switch (pgn) {
        case PGN_EEC1:
            if (ok16(le16(d + 3))) { out.rpm = le16(d + 3) * 0.125f; any = true; }
            break;
        case PGN_EEC2:
            if (ok8(d[2])) { out.loadPct = d[2]; any = true; }
            break;
        case PGN_ET1:
            if (ok8(d[0])) { out.coolantC = d[0] - 40.0f; any = true; }
            if (ok16(le16(d + 2))) { out.oilTempC = le16(d + 2) * 0.03125f - 273.0f; any = true; }
            break;
        case PGN_EFLP1:
            if (ok8(d[0])) { out.fuelPressKpa = d[0] * 4.0f; any = true; }
            if (ok8(d[3])) { out.oilPressKpa = d[3] * 4.0f; any = true; }
            if (ok8(d[6])) { out.coolantPressKpa = d[6] * 2.0f; any = true; }
            break;
        case PGN_LFE:
            if (ok16(le16(d))) { out.fuelRateLh = le16(d) * 0.05f; any = true; }
            break;
        case PGN_IC1:
            if (ok8(d[1])) { out.boostKpa = d[1] * 2.0f; any = true; }
            if (ok8(d[2])) { out.intakeC = d[2] - 40.0f; any = true; }
            break;
        case PGN_VEP1:
            if (ok16(le16(d + 4))) { out.moduleV = le16(d + 4) * 0.05f; any = true; }
            break;
        case PGN_DD:
            if (ok8(d[1])) { out.fuelPct = d[1] * 0.4f; any = true; }
            break;
        case PGN_HOURS: {
            const uint32_t h = (uint32_t)d[0] | ((uint32_t)d[1] << 8) | ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24);
            if (h <= 0xFAFFFFFFu) { out.engineHoursS = (uint32_t)((uint64_t)h * 180u); any = true; }   // 0,05 h por bit = 180 s
            break;
        }
        default: break;
    }
    return any ? pgn : 0;
}

// PGN que se escuchan (para la caducidad de cada grupo de datos)
inline const uint32_t* pgnList(int& n) {
    static const uint32_t L[] = { PGN_EEC1, PGN_EEC2, PGN_ET1, PGN_EFLP1, PGN_LFE, PGN_IC1, PGN_VEP1, PGN_DD, PGN_HOURS };
    n = (int)(sizeof(L) / sizeof(L[0]));
    return L;
}

// Borra de d lo que aporta un PGN que ha caducado
inline void clear(Obd::Data& d, uint32_t pgn) {
    switch (pgn) {
        case PGN_EEC1:  d.rpm = NAN; break;
        case PGN_EEC2:  d.loadPct = NAN; break;
        case PGN_ET1:   d.coolantC = NAN; d.oilTempC = NAN; break;
        case PGN_EFLP1: d.fuelPressKpa = NAN; d.oilPressKpa = NAN; d.coolantPressKpa = NAN; break;
        case PGN_LFE:   d.fuelRateLh = NAN; break;
        case PGN_IC1:   d.boostKpa = NAN; d.intakeC = NAN; break;
        case PGN_VEP1:  d.moduleV = NAN; break;
        case PGN_DD:    d.fuelPct = NAN; break;
        default: break;                                    // las horas no caducan: cambian muy despacio y a veces solo llegan al arrancar
    }
}

}  // namespace J1939
