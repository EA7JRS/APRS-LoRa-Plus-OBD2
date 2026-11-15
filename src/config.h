/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include "board.h"

// Configuración de la pasarela OBD-II → NMEA 2000. Se guarda en LittleFS (/obd_conf.json).
struct ObdGwConfig {
    std::string name = "APRS-OBD2";             // red (mDNS), punto de acceso y WebUI
    std::string pin = "123456";                  // clave de la WebUI (igual que el PIN de control del ecosistema; vacía = sin clave)

    // WiFi: 0 apagada, 1 cliente (con punto de acceso de rescate), 2 punto de acceso
    int         wifiMode = 2;
    std::string ssid, pass;
    std::string apPass = "aprsobd2";             // mínimo 8 caracteres

    // Bus del motor (MCP2515): "obd2" = coche (OBD-II, 500 kbit/s, consultas del modo 01);
    // "j1939" = motor de barco o industrial (SAE J1939, 250 kbit/s, solo escucha)
    std::string proto = "obd2";
    int         j1939Kbps = 250;                  // 250 (lo habitual) o 500
    int         mcpCs = OBD2_MCP_CS;
    int         mcpInt = OBD2_MCP_INT;
    int         mcpMHz = 16;                      // cristal del MCP2515 del kit: 16 u 8 (comprobar en la placa)
    int         pollMs = 200;                     // tiempo entre consultas OBD-II (una por turno)
    int         timeoutMs = 1500;                 // sin respuesta de la ECU durante este tiempo: el dato caduca

    // NMEA 2000 (TWAI nativo)
    int         canTx = OBD2_CAN_TX;
    int         canRx = OBD2_CAN_RX;
    int         n2kAddr = 35;                     // dirección preferida (se negocia en el bus y se guarda la que queda)
    int         engineInstance = 0;              // instancia del motor en los PGN (0 = babor / único)

    // Cómo se presentan algunos datos en NMEA 2000
    bool        mapAsBoost = true;               // presión del colector (MAP) como presión de sobrealimentación en 127488 (manométrica)
    double      fuelCapacityL = 0;               // capacidad del depósito en litros para 127505 (0 = sin dato)

    // Telemetría APRS (pendiente): de momento solo se guarda la preferencia
    bool        aprsOut = false;
};

extern ObdGwConfig Cfg;

namespace Config {
    void        begin();                          // monta LittleFS y lee /obd_conf.json
    bool        save();
    void        toJson(JsonObject o, bool secrets);
    // Aplica un JSON (parcial o entero); vacío = bien, si no el mensaje de error
    std::string apply(JsonObjectConst in, ObdGwConfig& out);
    void        reset();
}
