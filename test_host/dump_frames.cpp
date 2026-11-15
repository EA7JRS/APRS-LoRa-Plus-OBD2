// Copyright (C) 2026 Manuel - EA7JRS
// This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later.
// Para tools/canboat_check.py: sin argumentos escribe las tramas NMEA 2000 de nuestros codificadores con los valores que
// deben llevar (una línea JSON por PGN); con "j1939 <pgn> <hex>" decodifica una trama J1939 y escribe lo que ha leído.
#include "obd.h"
#include "j1939.h"
#include "n2k_engine.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void hex(const uint8_t* p, int n) { for (int i = 0; i < n; i++) printf("%02x", p[i]); }

static int n2k() {
    uint8_t b[8], f[26];
    N2kEng::engine127488(0, 2450.25f, 82300.0f, b);
    printf("{\"pgn\":127488,\"hex\":\""); hex(b, 8);
    printf("\",\"expect\":{\"Instance\":0,\"Speed\":2450.25,\"Boost Pressure\":82300,\"Tilt/Trim\":null}}\n");

    N2kEng::Dinamico d;
    d.oilPressPa = 412000; d.oilTempK = 371.6f; d.coolantK = 356.15f; d.altVoltageV = 13.84f; d.fuelRateLh = 7.3f;
    d.horasS = 1234567; d.coolantPressPa = 96000; d.fuelPressPa = 304000; d.cargaPct = 63; d.parPct = 0x7F;
    N2kEng::engine127489(1, d, f);
    printf("{\"pgn\":127489,\"hex\":\""); hex(f, 26);
    printf("\",\"expect\":{\"Instance\":1,\"Oil pressure\":412000,\"Oil temperature\":371.6,\"Temperature\":356.15,"
           "\"Alternator Potential\":13.84,\"Fuel Rate\":7.3,\"Total Engine hours\":1234567,\"Coolant Pressure\":96000,"
           "\"Fuel Pressure\":304000,\"Engine Load\":63,\"Engine Torque\":null}}\n");

    N2kEng::fluid127505(2, 0, 57.5f, 310.0f, b);
    printf("{\"pgn\":127505,\"hex\":\""); hex(b, 8);
    printf("\",\"expect\":{\"Instance\":2,\"Type\":0,\"Level\":57.5,\"Capacity\":310}}\n");

    N2kEng::battery127508(0, 12.62f, 9, b);
    printf("{\"pgn\":127508,\"hex\":\""); hex(b, 8);
    printf("\",\"expect\":{\"Instance\":0,\"Voltage\":12.62,\"Current\":null,\"Temperature\":null,\"SID\":9}}\n");
    return 0;
}

static void num(const char* k, float v, bool& first) {
    if (std::isnan(v)) return;
    printf("%s\"%s\":%.4f", first ? "" : ",", k, (double)v);
    first = false;
}

int main(int argc, char** argv) {
    if (argc == 1) return n2k();
    if (argc == 4 && !strcmp(argv[1], "j1939")) {
        const uint32_t pgn = (uint32_t)strtoul(argv[2], nullptr, 10);
        uint8_t d[8] = {};
        for (int i = 0; i < 8 && argv[3][2 * i] && argv[3][2 * i + 1]; i++) { char t[3] = {argv[3][2 * i], argv[3][2 * i + 1], 0}; d[i] = (uint8_t)strtoul(t, nullptr, 16); }
        Obd::Data o;
        const uint32_t id = (6u << 26) | (pgn << 8) | 0x00;       // prioridad 6, origen 0 (motor 1)
        const uint32_t got = J1939::decode(id, d, 8, o);
        bool first = true;
        printf("{\"pgn\":%u,", (unsigned)got);
        printf("\"v\":{");
        num("rpm", o.rpm, first); num("load", o.loadPct, first); num("coolantC", o.coolantC, first); num("oilTempC", o.oilTempC, first);
        num("fuelPressKpa", o.fuelPressKpa, first); num("oilPressKpa", o.oilPressKpa, first); num("coolantPressKpa", o.coolantPressKpa, first);
        num("fuelRateLh", o.fuelRateLh, first); num("boostKpa", o.boostKpa, first); num("intakeC", o.intakeC, first);
        num("volt", o.moduleV, first); num("fuelPct", o.fuelPct, first);
        if (o.engineHoursS != 0xFFFFFFFF) { printf("%s\"hoursS\":%u", first ? "" : ",", (unsigned)o.engineHoursS); }
        printf("}}\n");
        return 0;
    }
    return 2;
}
