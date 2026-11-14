/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// OBD-II (ISO 15765-4 / SAE J1979, CAN de 11 bits a 500 kbit/s): construye las peticiones del modo 01 y decodifica las
// respuestas de la ECU en magnitudes físicas. Solo consultas (nunca escribe en el bus del coche). Sin Arduino.
#pragma once
#include <stdint.h>
#include <cmath>

namespace Obd {

// PID del modo 01 que usamos
enum Pid : uint8_t {
    PID_LOAD = 0x04,        // carga del motor (%)
    PID_COOLANT = 0x05,     // temperatura del refrigerante (°C)
    PID_MAP = 0x0B,         // presión absoluta del colector (kPa)
    PID_RPM = 0x0C,         // régimen (rpm)
    PID_SPEED = 0x0D,       // velocidad del vehículo (km/h)
    PID_INTAKE = 0x0F,      // temperatura del aire de admisión (°C)
    PID_RUNTIME = 0x1F,     // tiempo de marcha desde el arranque (s)
    PID_FUEL_LEVEL = 0x2F,  // nivel de combustible (%)
    PID_MODULE_V = 0x42,    // tensión del módulo de control (V)
    PID_OIL_TEMP = 0x5C,    // temperatura del aceite (°C)
    PID_FUEL_RATE = 0x5E,   // consumo instantáneo (L/h)
};

// Petición funcional OBD-II: CAN id 0x7DF, datos [02 01 PID 00..]. Devuelve 8 bytes en out.
inline void request(uint8_t pid, uint8_t out[8]) {
    out[0] = 0x02; out[1] = 0x01; out[2] = pid;
    out[3] = out[4] = out[5] = out[6] = out[7] = 0x55;   // relleno recomendado (0x55 / 0xAA / 0x00)
}
inline uint32_t requestId() { return 0x7DF; }

// Magnitudes decodificadas (NAN / -1 = sin dato)
struct Data {
    float rpm = NAN;
    float speedKmh = NAN;
    float coolantC = NAN;
    float oilTempC = NAN;
    float intakeC = NAN;
    float fuelPct = NAN;
    float moduleV = NAN;
    float loadPct = NAN;
    float fuelRateLh = NAN;
    float mapKpa = NAN;
    uint32_t runtimeS = 0xFFFFFFFF;
    bool has(float v) const { return !std::isnan(v); }
};

// Decodifica una respuesta de la ECU (id 0x7E8..0x7EF): [n, 0x41, PID, A, B, ...]. Devuelve el PID decodificado o 0 si no vale.
inline uint8_t decode(const uint8_t* d, int len, Data& out) {
    if (len < 3 || d[1] != 0x41) return 0;                 // 0x41 = respuesta al modo 01
    const uint8_t pid = d[2];
    const uint8_t A = len > 3 ? d[3] : 0;
    const uint8_t B = len > 4 ? d[4] : 0;
    switch (pid) {
        case PID_LOAD:       out.loadPct = A * 100.0f / 255.0f; break;
        case PID_COOLANT:    out.coolantC = A - 40.0f; break;
        case PID_MAP:        out.mapKpa = A; break;
        case PID_RPM:        out.rpm = (A * 256 + B) / 4.0f; break;
        case PID_SPEED:      out.speedKmh = A; break;
        case PID_INTAKE:     out.intakeC = A - 40.0f; break;
        case PID_RUNTIME:    out.runtimeS = (uint32_t)(A * 256 + B); break;
        case PID_FUEL_LEVEL: out.fuelPct = A * 100.0f / 255.0f; break;
        case PID_MODULE_V:   out.moduleV = (A * 256 + B) / 1000.0f; break;
        case PID_OIL_TEMP:   out.oilTempC = A - 40.0f; break;
        case PID_FUEL_RATE:  out.fuelRateLh = (A * 256 + B) / 20.0f; break;
        default: return 0;
    }
    return pid;
}

// Lista de PID que la pasarela consulta por turnos (los que un plotter muestra como indicadores de motor)
inline const uint8_t* pollList(int& n) {
    static const uint8_t L[] = { PID_RPM, PID_COOLANT, PID_SPEED, PID_LOAD, PID_MODULE_V, PID_FUEL_LEVEL,
                                 PID_OIL_TEMP, PID_FUEL_RATE, PID_RUNTIME, PID_MAP, PID_INTAKE };
    n = (int)(sizeof(L) / sizeof(L[0]));
    return L;
}

}  // namespace Obd
