/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */

#include "n2k.h"
#include "config.h"
#include "n2k_engine.h"
#include "driver/twai.h"
#include <Preferences.h>
#include <cmath>

namespace N2k {

    static Status st;
    static Preferences prefs;
    static N2kEng::AddressClaim* claim = nullptr;
    static uint32_t last88 = 0, last89 = 0, last05 = 0, last08 = 0, lastClaimSave = 0;
    static uint8_t fpSeq = 0, sid = 0;

    static const uint32_t PGN_88 = 127488, PGN_89 = 127489, PGN_05 = 127505, PGN_08 = 127508;

    static void tx(uint32_t id, const uint8_t* d, int len) {
        twai_message_t m = {};
        m.extd = 1;
        m.identifier = id;
        m.data_length_code = (uint8_t)len;
        memcpy(m.data, d, (size_t)len);
        if (twai_transmit(&m, 0) == ESP_OK) st.sent++;
    }
    static void txFrame(const N2kEng::AddressClaim::Frame& f) { tx(f.id, f.d, f.len); }

    void begin() {
        prefs.begin("obd2n2k", false);
        const uint8_t addr = (uint8_t)prefs.getUChar("addr", (uint8_t)Cfg.n2kAddr);

        const uint64_t efuse = ESP.getEfuseMac();            // número único del NAME desde la MAC de fábrica
        uint32_t unico = (uint32_t)(efuse & 0x1FFFFF);
        claim = new N2kEng::AddressClaim(N2kEng::name(unico), addr);

        twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)Cfg.canTx, (gpio_num_t)Cfg.canRx, TWAI_MODE_NORMAL);
        g.rx_queue_len = 32;
        g.tx_queue_len = 16;
        twai_timing_config_t t = TWAI_TIMING_CONFIG_250KBITS();
        twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        if (twai_driver_install(&g, &t, &f) != ESP_OK) { Serial.println("[n2k] TWAI: no se pudo instalar el controlador"); return; }
        if (twai_start() != ESP_OK) { twai_driver_uninstall(); Serial.println("[n2k] TWAI: no arranca"); return; }
        st.running = true;
        txFrame(claim->arrancar(millis()));                  // primer reclamo de dirección
        st.address = claim->address();
        Serial.printf("[n2k] NMEA 2000 a 250 kbit/s (TX %d, RX %d), dirección preferida %u\n", Cfg.canTx, Cfg.canRx, addr);
    }

    static int8_t pct8(float v) {
        if (std::isnan(v)) return (int8_t)0x7F;
        long n = lroundf(v);
        if (n > 125) n = 125; if (n < -125) n = -125;
        return (int8_t)n;
    }
    static float toK(float c) { return std::isnan(c) ? NAN : c + 273.15f; }

    void loop(const Obd::Data& d) {
        if (!st.running || !claim) return;
        const uint32_t now = millis();

        // recibir: reclamos y peticiones de otros nodos (defender / negociar la dirección)
        twai_message_t m;
        int budget = 24;
        while (budget-- > 0 && twai_receive(&m, 0) == ESP_OK) {
            if (!m.extd) continue;
            st.received++;
            N2kEng::AddressClaim::Frame out;
            if (claim->alRecibir(m.identifier, m.data, m.data_length_code, now, out)) txFrame(out);
        }
        st.address = claim->address();
        st.lost = claim->perdida();
        st.claims = claim->cambios();

        // recuperación del bus
        twai_status_info_t info;
        if (twai_get_status_info(&info) == ESP_OK) {
            if (info.state == TWAI_STATE_BUS_OFF) twai_initiate_recovery();
            else if (info.state == TWAI_STATE_STOPPED) twai_start();
        }

        if (!claim->listo(now)) { st.claimed = false; return; }   // aún negociando la dirección
        st.claimed = true;
        const uint8_t src = claim->address();
        const uint8_t inst = (uint8_t)Cfg.engineInstance;

        // guardar la dirección negociada cuando cambie
        if (now - lastClaimSave > 5000) {
            lastClaimSave = now;
            if (prefs.getUChar("addr", 255) != src) prefs.putUChar("addr", src);
        }

        // 127488 motor rápido (rpm, sobrealimentación): 10 veces por segundo
        if (now - last88 >= 100 && (!std::isnan(d.rpm) || !std::isnan(d.mapKpa))) {
            last88 = now;
            float boostPa = NAN;
            if (Cfg.mapAsBoost && !std::isnan(d.mapKpa)) { float g = d.mapKpa - 101.325f; boostPa = (g > 0 ? g : 0) * 1000.0f; }
            uint8_t p[8];
            N2kEng::engine127488(inst, d.rpm, boostPa, p);
            tx(N2kEng::makeId(2, PGN_88, src), p, 8);
        }

        // 127489 motor dinámico (temperaturas, consumo, tensión, horas, carga): 2 veces por segundo, fast packet
        const bool has89 = !std::isnan(d.coolantC) || !std::isnan(d.oilTempC) || !std::isnan(d.moduleV) ||
                           !std::isnan(d.fuelRateLh) || !std::isnan(d.loadPct) || d.runtimeS != 0xFFFFFFFF;
        if (now - last89 >= 500 && has89) {
            last89 = now;
            N2kEng::Dinamico dy;
            dy.coolantK = toK(d.coolantC);
            dy.oilTempK = toK(d.oilTempC);
            dy.altVoltageV = d.moduleV;
            dy.fuelRateLh = d.fuelRateLh;
            dy.horasS = d.runtimeS;
            dy.cargaPct = pct8(d.loadPct);
            uint8_t payload[26];
            N2kEng::engine127489(inst, dy, payload);
            uint8_t frames[6 * 8];
            const int nf = N2kEng::fastFrames(payload, 26, fpSeq, frames, sizeof(frames));
            fpSeq = (uint8_t)((fpSeq + 1) & 7);
            for (int i = 0; i < nf; i++) tx(N2kEng::makeId(2, PGN_89, src), &frames[i * 8], 8);
        }

        // 127505 nivel de combustible: cada 2 s
        if (now - last05 >= 2000 && !std::isnan(d.fuelPct)) {
            last05 = now;
            uint8_t p[8];
            N2kEng::fluid127505(inst, 0, d.fuelPct, Cfg.fuelCapacityL > 0 ? (float)Cfg.fuelCapacityL : NAN, p);
            tx(N2kEng::makeId(6, PGN_05, src), p, 8);
        }

        // 127508 batería (tensión del módulo): cada segundo
        if (now - last08 >= 1000 && !std::isnan(d.moduleV)) {
            last08 = now;
            uint8_t p[8];
            N2kEng::battery127508(inst, d.moduleV, sid++, p);
            tx(N2kEng::makeId(6, PGN_08, src), p, 8);
        }
    }

    const Status& status() { return st; }
}
