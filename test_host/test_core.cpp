// Copyright (C) 2026 Manuel - EA7JRS
// This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later.
#include "obd.h"
#include "n2k_engine.h"
#include "j1939.h"
#include <cstdio>
#include <cmath>

static int checks = 0, fails = 0;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)
static bool near(double a, double b, double e = 1e-3) { return std::fabs(a - b) <= e; }
static uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const uint8_t* p) { return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24)); }

static void testObd() {
    uint8_t req[8];
    Obd::request(Obd::PID_RPM, req);
    CHECK(req[0] == 0x02 && req[1] == 0x01 && req[2] == 0x0C && Obd::requestId() == 0x7DF);
    Obd::Data d;
    // respuesta de RPM: n=4, 41 0C A B  -> (256A+B)/4
    uint8_t rpm[] = {0x04, 0x41, 0x0C, 0x1A, 0xF8};       // 0x1AF8 = 6904 /4 = 1726
    CHECK(Obd::decode(rpm, 5, d) == 0x0C && near(d.rpm, (0x1A * 256 + 0xF8) / 4.0));
    uint8_t cool[] = {0x03, 0x41, 0x05, 90};             // 90-40 = 50 C
    CHECK(Obd::decode(cool, 4, d) == 0x05 && near(d.coolantC, 50));
    uint8_t spd[] = {0x03, 0x41, 0x0D, 100};
    CHECK(Obd::decode(spd, 4, d) == 0x0D && near(d.speedKmh, 100));
    uint8_t volt[] = {0x04, 0x41, 0x42, 0x37, 0x70};     // 0x3770 = 14192 /1000 = 14.192 V
    CHECK(Obd::decode(volt, 5, d) == 0x42 && near(d.moduleV, 14.192, 1e-3));
    uint8_t fuel[] = {0x03, 0x41, 0x2F, 0x80};           // 128*100/255 = 50.2 %
    CHECK(Obd::decode(fuel, 4, d) == 0x2F && near(d.fuelPct, 128 * 100.0 / 255.0, 0.01));
    uint8_t oil[] = {0x03, 0x41, 0x5C, 120};             // 120-40 = 80 C
    CHECK(Obd::decode(oil, 4, d) == 0x5C && near(d.oilTempC, 80));
    uint8_t rate[] = {0x04, 0x41, 0x5E, 0x00, 0x64};     // 100/20 = 5 L/h
    CHECK(Obd::decode(rate, 5, d) == 0x5E && near(d.fuelRateLh, 5));
    uint8_t mala[] = {0x03, 0x42, 0x05, 90};             // 0x42 no es respuesta al modo 01
    CHECK(Obd::decode(mala, 4, d) == 0);
    int n = 0; Obd::pollList(n); CHECK(n >= 8);
}

static void testEngineN2k() {
    using namespace N2kEng;
    CHECK(makeId(2, 127488, 0x20) == 0x09F20020u);
    CHECK(makeId(6, 60928, 0x20, 255) == 0x18EEFF20u);
    uint8_t m[8];
    engine127488(0, 1726.0f, 120000.0f, m);
    CHECK(m[0] == 0 && rd16(&m[1]) == (uint16_t)lroundf(1726 / 0.25f) && rd16(&m[3]) == 1200 && m[5] == 0x7F);
    Dinamico din; din.coolantK = 323.15f; din.oilTempK = 353.15f; din.altVoltageV = 14.2f; din.fuelRateLh = 5.0f; din.horasS = 3600; din.cargaPct = 42;
    uint8_t e[26];
    engine127489(3, din, e);
    CHECK(e[0] == 3 && rd16(&e[5]) == (uint16_t)lroundf(323.15f / 0.01f) && rd16(&e[3]) == (uint16_t)lroundf(353.15f / 0.1f));
    CHECK((int16_t)rd16(&e[7]) == (int16_t)lroundf(14.2f / 0.01f) && (int16_t)rd16(&e[9]) == 50 && rd32(&e[11]) == 3600 && e[24] == 42);
    uint8_t f[8];
    fluid127505(0, 0, 50.0f, 60.0f, f);
    CHECK((f[0] & 0x0F) == 0 && (int16_t)rd16(&f[1]) == (int16_t)lroundf(50.0f / 0.004f) && rd32(&f[3]) == 600);
    uint8_t b[8];
    battery127508(0, 12.6f, 7, b);
    CHECK((int16_t)rd16(&b[1]) == 1260 && b[7] == 7);
    // fast packet de 127489 (26 bytes)
    uint8_t fr[8 * 5]; int nf = fastFrames(e, 26, 2, fr, sizeof fr);
    CHECK(nf == 4 && fr[0] == ((2 << 5) | 0) && fr[1] == 26 && fr[2] == e[0] && fr[8] == ((2 << 5) | 1) && fr[9] == e[6]);
    // reclamo de dirección
    AddressClaim ac(name(0x123), 50);
    AddressClaim::Frame fa = ac.arrancar(0);
    CHECK(parseId(fa.id).pgn == 60928 && ac.address() == 50 && !ac.listo(100) && ac.listo(300));
    uint8_t bajo[8] = {0};                                // NAME 0: el más bajo gana
    AddressClaim::Frame out;
    CHECK(ac.alRecibir(makeId(6, 60928, 50, 255), bajo, 8, 400, out) && ac.address() != 50 && ac.cambios() == 1);
}

static void testJ1939() {
    Obd::Data d;
    const uint32_t id = 0x0CF00400;                                   // EEC1 del motor (origen 0), prioridad 3
    CHECK(J1939::pgnOf(id) == 61444 && J1939::sourceOf(id) == 0);
    CHECK(J1939::pgnOf(0x18EAFF00) == 59904);                         // PDU1: el destino no entra en el PGN
    uint8_t eec1[8] = {0xFF, 0xFF, 0xFF, 0x68, 0x13, 0xFF, 0xFF, 0xFF};   // 0x1368 = 4968 * 0,125 = 621 rpm
    CHECK(J1939::decode(id, eec1, 8, d) == 61444 && near(d.rpm, 621.0));
    uint8_t et1[8] = {0x7B, 0xFF, 0x40, 0x2E, 0xFF, 0xFF, 0xFF, 0xFF};    // 123 - 40 = 83 C; 0x2E40 = 11840 / 32 - 273 = 97 C
    CHECK(J1939::decode(0x18FEEE00, et1, 8, d) == 65262 && near(d.coolantC, 83) && near(d.oilTempC, 97));
    uint8_t nada[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    Obd::Data e;
    CHECK(J1939::decode(0x18FEEE00, nada, 8, e) == 0 && std::isnan(e.coolantC));     // todo "sin dato": no se toma nada
    CHECK(J1939::decode(0x18FEEE00, et1, 7, e) == 0);                                 // trama corta
    uint8_t err[8] = {0xFE, 0xFF, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF};                // 0xFE / 0xFExx = error del sensor
    CHECK(J1939::decode(0x18FEEE00, err, 8, e) == 0 && std::isnan(e.oilTempC));
    J1939::clear(d, 65262);
    CHECK(std::isnan(d.coolantC) && std::isnan(d.oilTempC) && near(d.rpm, 621.0));
}

int main() {
    testObd();
    testJ1939();
    testEngineN2k();
    printf("%d comprobaciones, %d fallos\n", checks, fails);
    return fails ? 1 : 0;
}
