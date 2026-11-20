/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */

#include "obd_link.h"
#include "config.h"
#include "j1939.h"
#include <SPI.h>
#include <mcp_can.h>

namespace ObdLink {

    static MCP_CAN* can = nullptr;
    static bool     ok = false;
    static Obd::Data data;                       // último valor decodificado de cada magnitud
    static uint32_t pidAt[32] = {};              // ms del último dato de cada PID de la lista (índice = posición en pollList)
    static uint32_t lastPoll = 0, lastRx = 0;
    static int      next = 0;
    static uint32_t reqN = 0, respN = 0, errN = 0;
    static bool     j1939 = false;               // motor J1939: solo se escucha
    static uint32_t pgnAt[16] = {};              // ms del último dato de cada PGN J1939 (índice = posición en pgnList)

    void begin() {
        SPI.begin(7, 8, 9, Cfg.mcpCs);           // XIAO ESP32-S3: SCK D8=7, MISO D9=8, MOSI D10=9
        can = new MCP_CAN(Cfg.mcpCs);
        const byte clk = Cfg.mcpMHz == 8 ? MCP_8MHz : MCP_16MHz;
        j1939 = Cfg.proto == "j1939";
        const int kbps = j1939 ? Cfg.j1939Kbps : 500;
        ok = can->begin(kbps == 250 ? CAN_250KBPS : CAN_500KBPS, clk) == CAN_OK;   // begin() deja el MCP2515 en modo normal
        Serial.printf("[obd] MCP2515 (CS %d, %d MHz), %s a %d kbit/s: %s\n", Cfg.mcpCs, Cfg.mcpMHz, j1939 ? "J1939" : "OBD-II", kbps,
                      ok ? "OK" : "no responde");
    }

    // Borra en la copia el campo del PID que ha caducado
    static void clearField(Obd::Data& d, uint8_t pid) {
        switch (pid) {
            case Obd::PID_RPM:        d.rpm = NAN; break;
            case Obd::PID_SPEED:      d.speedKmh = NAN; break;
            case Obd::PID_COOLANT:    d.coolantC = NAN; break;
            case Obd::PID_OIL_TEMP:   d.oilTempC = NAN; break;
            case Obd::PID_INTAKE:     d.intakeC = NAN; break;
            case Obd::PID_FUEL_LEVEL: d.fuelPct = NAN; break;
            case Obd::PID_MODULE_V:   d.moduleV = NAN; break;
            case Obd::PID_LOAD:       d.loadPct = NAN; break;
            case Obd::PID_FUEL_RATE:  d.fuelRateLh = NAN; break;
            case Obd::PID_MAP:        d.mapKpa = NAN; break;
            case Obd::PID_RUNTIME:    d.runtimeS = 0xFFFFFFFF; break;
            default: break;
        }
    }

    void loop() {
        if (!ok) return;
        const uint32_t now = millis();

        // recibir las respuestas de la ECU (0x7E8..0x7EF)
        while (can->checkReceive() == CAN_MSGAVAIL) {
            unsigned long id = 0; byte len = 0; byte buf[8] = {};
            if (can->readMsgBufID(&id, &len, buf) != CAN_OK) { errN++; break; }
            if (j1939) {                                     // motor J1939: tramas de 29 bits que el motor difunde solo
                if (!can->isExtendedFrame()) continue;
                const uint32_t pgn = J1939::decode((uint32_t)id, buf, len, data);
                if (!pgn) continue;
                respN++;
                lastRx = now;
                int m = 0; const uint32_t* P = J1939::pgnList(m);
                for (int i = 0; i < m; i++) if (P[i] == pgn) { pgnAt[i] = now; break; }
                continue;
            }
            if (can->isExtendedFrame() || id < 0x7E8 || id > 0x7EF) continue;   // no es una respuesta OBD-II
            const uint8_t pid = Obd::decode(buf, len, data);
            if (!pid) continue;
            respN++;
            lastRx = now;
            int n = 0; const uint8_t* L = Obd::pollList(n);
            for (int i = 0; i < n; i++) if (L[i] == pid) { pidAt[i] = now; break; }
        }

        // enviar la siguiente consulta por turnos (con J1939 no se envía nada)
        if (!j1939 && now - lastPoll >= (uint32_t)Cfg.pollMs) {
            lastPoll = now;
            int n = 0; const uint8_t* L = Obd::pollList(n);
            if (n > 0) {
                uint8_t req[8];
                Obd::request(L[next % n], req);
                if (can->sendMsgBuf(Obd::requestId(), 0, 8, req) == CAN_OK) reqN++;
                else errN++;
                next = (next + 1) % n;
            }
        }
    }

    Obd::Data snapshot() {
        Obd::Data d = data;
        const uint32_t now = millis();
        if (j1939) {
            int m = 0; const uint32_t* P = J1939::pgnList(m);
            for (int i = 0; i < m; i++)
                if (pgnAt[i] == 0 || now - pgnAt[i] > (uint32_t)Cfg.timeoutMs * 4) J1939::clear(d, P[i]);   // difunden hasta cada 1 s
            return d;
        }
        int n = 0; const uint8_t* L = Obd::pollList(n);
        for (int i = 0; i < n; i++)
            if (pidAt[i] == 0 || now - pidAt[i] > (uint32_t)Cfg.timeoutMs) clearField(d, L[i]);
        return d;
    }

    bool     busOk()    { return ok; }
    bool     ecuAlive() { return ok && lastRx != 0 && millis() - lastRx < (uint32_t)Cfg.timeoutMs * (j1939 ? 4u : 1u); }
    uint32_t requests() { return reqN; }
    uint32_t responses() { return respN; }
    uint32_t rxErrors() { return errN; }
}
