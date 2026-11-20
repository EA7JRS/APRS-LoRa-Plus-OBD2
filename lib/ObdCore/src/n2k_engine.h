/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// Emisión NMEA 2000 de los datos de motor (ISO 11783-3 / NMEA 2000, 250 kbit/s): identificador de 29 bits, NAME, reclamo de
// dirección ISO (60928 / 59904) y codificadores de los PGN de motor que muestran los plotters (APRS LoRa+ y Garmin):
// 127488 (rápido), 127489 (dinámico, fast packet), 127505 (nivel de combustible) y 127508 (batería). Formatos contrastados
// con la base de datos de CANboat (https://canboat.github.io/canboat/canboat.html, canboat.json v9.2.0, Apache-2.0;
// tools/canboat_run.sh lo repite en la CI) y con la librería NMEA2000. Sin Arduino. No es un nodo certificado.
#pragma once
#include <stdint.h>
#include <string.h>
#include <cmath>

namespace N2kEng {

static const uint32_t PGN_PETICION = 59904, PGN_RECLAMO = 60928;

inline uint32_t makeId(uint8_t prio, uint32_t pgn, uint8_t src, uint8_t dst = 255) {
    uint32_t pf = (pgn >> 8) & 0xFF;
    uint32_t p = pf < 240 ? ((pgn & 0x3FF00) | dst) : (pgn & 0x3FFFF);
    return ((uint32_t)(prio & 7) << 26) | (p << 8) | src;
}
struct Id { uint8_t prio, src, dst; uint32_t pgn; };
inline Id parseId(uint32_t id) {
    Id r; r.prio = (id >> 26) & 7; r.src = id & 0xFF;
    uint8_t ps = (id >> 8) & 0xFF, pf = (id >> 16) & 0xFF, dp = (id >> 24) & 1;
    r.pgn = pf < 240 ? ((uint32_t)dp << 16) | (pf << 8) : ((uint32_t)dp << 16) | (pf << 8) | ps;
    r.dst = pf < 240 ? ps : 255;
    return r;
}

inline void u16(uint8_t* p, uint16_t v) { p[0] = v & 0xFF; p[1] = v >> 8; }
inline void u32(uint8_t* p, uint32_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = v >> 24; }
inline void u16res(uint8_t* p, float v, float res) { if (std::isnan(v)) { u16(p, 0xFFFF); return; } long n = lroundf(v / res); if (n < 0) n = 0; if (n > 0xFFFD) n = 0xFFFD; u16(p, (uint16_t)n); }
inline void i16res(uint8_t* p, float v, float res) { if (std::isnan(v)) { u16(p, 0x7FFF); return; } long n = lroundf(v / res); if (n > 32766) n = 32766; if (n < -32767) n = -32767; u16(p, (uint16_t)(int16_t)n); }

// 127488 motor rápido: rpm, presión de turbo (Pa). trim -128..127 (0x7F sin dato).
inline void engine127488(uint8_t inst, float rpm, float boostPa, uint8_t out[8]) {
    memset(out, 0xFF, 8);
    out[0] = inst;
    u16res(&out[1], rpm, 0.25f);
    u16res(&out[3], boostPa, 100.0f);
    out[5] = 0x7F;                       // trim: sin dato
}

struct Dinamico {
    float oilPressPa = NAN, oilTempK = NAN, coolantK = NAN, altVoltageV = NAN, fuelRateLh = NAN;
    float coolantPressPa = NAN, fuelPressPa = NAN;
    uint32_t horasS = 0xFFFFFFFF;
    int8_t cargaPct = 0x7F, parPct = 0x7F;
};
// 127489 motor dinámico (fast packet, 26 bytes)
inline void engine127489(uint8_t inst, const Dinamico& d, uint8_t out[26]) {
    memset(out, 0xFF, 26);
    out[0] = inst;
    u16res(&out[1], d.oilPressPa, 100.0f);
    u16res(&out[3], d.oilTempK, 0.1f);
    u16res(&out[5], d.coolantK, 0.01f);
    i16res(&out[7], d.altVoltageV, 0.01f);
    i16res(&out[9], d.fuelRateLh, 0.1f);
    u32(&out[11], d.horasS);
    u16res(&out[15], d.coolantPressPa, 100.0f);
    u16res(&out[17], d.fuelPressPa, 1000.0f);
    out[19] = 0xFF;
    u16(&out[20], 0);                    // estado 1
    u16(&out[22], 0);                    // estado 2
    out[24] = (uint8_t)d.cargaPct;
    out[25] = (uint8_t)d.parPct;
}

// 127505 nivel de líquido. fluidType 0 = combustible. levelPct 0..100; capacityL litros (NAN = sin dato).
inline void fluid127505(uint8_t inst, uint8_t fluidType, float levelPct, float capacityL, uint8_t out[8]) {
    memset(out, 0xFF, 8);
    out[0] = (inst & 0x0F) | ((fluidType & 0x0F) << 4);
    i16res(&out[1], levelPct, 0.004f);
    u32(&out[3], std::isnan(capacityL) ? 0xFFFFFFFF : (uint32_t)lroundf(capacityL / 0.1f));
}

// 127508 batería: tensión (V), SID.
inline void battery127508(uint8_t inst, float voltageV, uint8_t sid, uint8_t out[8]) {
    memset(out, 0xFF, 8);
    out[0] = inst;
    i16res(&out[1], voltageV, 0.01f);
    u16(&out[3], 0x7FFF);                // corriente: sin dato
    u16(&out[5], 0xFFFF);                // temperatura: sin dato
    out[7] = sid;
}

// Parte un payload largo en tramas de fast packet. seq 0..7. out: n*8 bytes. Devuelve n (o -1 si no cabe).
inline int fastFrames(const uint8_t* payload, int len, uint8_t seq, uint8_t* out, int outCap) {
    int frames = len <= 6 ? 1 : 1 + (len - 6 + 6) / 7;
    if (frames * 8 > outCap) return -1;
    uint8_t s = (uint8_t)((seq & 7) << 5);
    int pos = 0;
    for (int f = 0; f < frames; f++) {
        uint8_t* fr = out + f * 8;
        memset(fr, 0xFF, 8);
        fr[0] = s | (uint8_t)f;
        int b = 1;
        if (f == 0) fr[b++] = (uint8_t)len;
        while (b < 8 && pos < len) fr[b++] = payload[pos++];
    }
    return frames;
}

// NAME de 64 bits (clase 50 = propulsión, función 160 = motor; fabricante 2046 sin registrar; grupo 4 marina)
inline uint64_t name(uint32_t unico, uint8_t clase = 50, uint8_t funcion = 160) {
    uint64_t v = 0;
    v |= (uint64_t)(unico & 0x1FFFFF);
    v |= (uint64_t)(2046 & 0x7FF) << 21;
    v |= (uint64_t)funcion << 40;
    v |= (uint64_t)(clase & 0x7F) << 49;
    v |= (uint64_t)(4 & 0x07) << 60;
    v |= (uint64_t)1 << 63;              // dirección arbitraria
    return v;
}

// Reclamo de dirección ISO: arranca, defiende y negocia.
class AddressClaim {
   public:
    struct Frame { uint32_t id = 0; uint8_t d[8] = {}; int len = 0; };
    AddressClaim(uint64_t name, uint8_t preferida) : name_(name), addr_(preferida > 251 ? 0 : preferida) {}
    uint8_t address() const { return addr_; }
    bool perdida() const { return addr_ == 254; }
    Frame arrancar(uint32_t now) { started_ = true; claimedMs_ = now; Frame f; reclamo(f); return f; }
    bool listo(uint32_t now) const { return started_ && !perdida() && now - claimedMs_ >= 250; }
    uint32_t cambios() const { return cambios_; }
    bool alRecibir(uint32_t id, const uint8_t* d, int len, uint32_t now, Frame& out) {
        if (!started_) return false;
        Id p = parseId(id);
        if (p.pgn == PGN_PETICION) {
            if (len >= 3) { uint32_t pgn = d[0] | (d[1] << 8) | ((uint32_t)d[2] << 16);
                if (pgn == PGN_RECLAMO && (p.dst == 255 || p.dst == addr_)) { claimedMs_ = now; reclamo(out); return true; } }
            return false;
        }
        if (p.pgn != PGN_RECLAMO || len < 8) return false;
        if (p.src < 252) taken_[p.src >> 3] |= (uint8_t)(1 << (p.src & 7));
        if (p.src != addr_) return false;
        uint64_t otro = 0; for (int i = 7; i >= 0; i--) otro = (otro << 8) | d[i];
        if (name_ < otro) { claimedMs_ = now; reclamo(out); return true; }
        cambios_++;
        if (!siguiente()) return false;
        claimedMs_ = now; reclamo(out); return true;
    }

   private:
    void reclamo(Frame& f) const { f.id = makeId(6, PGN_RECLAMO, addr_, 255); f.len = 8; uint64_t v = name_; for (int i = 0; i < 8; i++) { f.d[i] = v & 0xFF; v >>= 8; } }
    bool siguiente() { if (!(name_ >> 63)) { addr_ = 254; return false; } for (int a = 128; a <= 251; a++) if (!(taken_[a >> 3] & (1 << (a & 7)))) { addr_ = (uint8_t)a; return true; } addr_ = 254; return false; }
    uint64_t name_;
    uint8_t addr_;
    uint32_t claimedMs_ = 0, cambios_ = 0;
    bool started_ = false;
    uint8_t taken_[32] = {};
};

}  // namespace N2kEng
