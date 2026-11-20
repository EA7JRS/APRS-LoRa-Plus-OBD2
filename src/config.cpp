/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */

#include "config.h"
#include <LittleFS.h>

ObdGwConfig Cfg;

namespace Config {

    static const char* PATH = "/obd_conf.json";
    static const char* MASK = "********";

    template <typename T> static void get(JsonObjectConst o, const char* k, T& v) { if (o[k].is<T>()) v = o[k].as<T>(); }
    static void getS(JsonObjectConst o, const char* k, std::string& v) { if (o[k].is<const char*>()) v = o[k].as<const char*>(); }
    static void getSecret(JsonObjectConst o, const char* k, std::string& v) {
        if (o[k].is<const char*>() && strcmp(o[k].as<const char*>(), MASK) != 0) v = o[k].as<const char*>();
    }

    void toJson(JsonObject o, bool secrets) {
        const ObdGwConfig& c = Cfg;
        o["name"] = c.name; o["pin"] = secrets ? c.pin : (c.pin.empty() ? "" : MASK);
        JsonObject w = o["wifi"].to<JsonObject>();
        w["mode"] = c.wifiMode; w["ssid"] = c.ssid; w["pass"] = secrets ? c.pass : MASK; w["apPass"] = secrets ? c.apPass : MASK;
        JsonObject b = o["obd"].to<JsonObject>();
        b["proto"] = c.proto; b["j1939Kbps"] = c.j1939Kbps; b["cs"] = c.mcpCs; b["int"] = c.mcpInt; b["mhz"] = c.mcpMHz; b["pollMs"] = c.pollMs; b["timeoutMs"] = c.timeoutMs;
        JsonObject n = o["n2k"].to<JsonObject>();
        n["tx"] = c.canTx; n["rx"] = c.canRx; n["addr"] = c.n2kAddr; n["instance"] = c.engineInstance;
        n["mapAsBoost"] = c.mapAsBoost; n["fuelCapacityL"] = c.fuelCapacityL;
        JsonObject a = o["aprs"].to<JsonObject>();
        a["out"] = c.aprsOut;
    }

    std::string apply(JsonObjectConst in, ObdGwConfig& n) {
        getS(in, "name", n.name); getSecret(in, "pin", n.pin);
        JsonObjectConst w = in["wifi"];
        get(w, "mode", n.wifiMode); getS(w, "ssid", n.ssid); getSecret(w, "pass", n.pass); getSecret(w, "apPass", n.apPass);
        JsonObjectConst b = in["obd"];
        getS(b, "proto", n.proto); get(b, "j1939Kbps", n.j1939Kbps); get(b, "cs", n.mcpCs); get(b, "int", n.mcpInt); get(b, "mhz", n.mcpMHz); get(b, "pollMs", n.pollMs); get(b, "timeoutMs", n.timeoutMs);
        JsonObjectConst k = in["n2k"];
        get(k, "tx", n.canTx); get(k, "rx", n.canRx); get(k, "addr", n.n2kAddr); get(k, "instance", n.engineInstance);
        get(k, "mapAsBoost", n.mapAsBoost);
        if (k["fuelCapacityL"].is<double>()) n.fuelCapacityL = k["fuelCapacityL"].as<double>();
        JsonObjectConst a = in["aprs"];
        get(a, "out", n.aprsOut);

        if (n.name.empty() || n.name.size() > 24) return "name 1-24";
        if (n.wifiMode < 0 || n.wifiMode > 2) return "wifi.mode 0-2";
        if (n.apPass.size() < 8) return "wifi.apPass >= 8";
        if (n.proto != "obd2" && n.proto != "j1939") return "obd.proto obd2 | j1939";
        if (n.j1939Kbps != 250 && n.j1939Kbps != 500) return "obd.j1939Kbps 250 | 500";
        if (n.mcpMHz != 8 && n.mcpMHz != 16) return "obd.mhz 8 | 16";
        if (n.mcpCs < 0 || n.mcpCs > 48) return "obd.cs";
        if (n.pollMs < 20 || n.pollMs > 2000) return "obd.pollMs 20-2000";
        if (n.timeoutMs < 200 || n.timeoutMs > 20000) return "obd.timeoutMs 200-20000";
        if (n.canTx < 0 || n.canTx > 48) return "n2k.tx";
        if (n.canRx < 0 || n.canRx > 48) return "n2k.rx";
        if (n.canTx == n.canRx) return "n2k.tx = n2k.rx";
        if (n.n2kAddr < 0 || n.n2kAddr > 251) return "n2k.addr 0-251";
        if (n.engineInstance < 0 || n.engineInstance > 252) return "n2k.instance 0-252";
        if (n.fuelCapacityL < 0 || n.fuelCapacityL > 100000) return "n2k.fuelCapacityL";
        for (int used : {7, 8, 9}) if (n.canTx == used || n.canRx == used) return "n2k.tx / n2k.rx (pin de SPI)";
        if (n.canTx == n.mcpCs || n.canRx == n.mcpCs) return "n2k.tx / n2k.rx (CS del MCP2515)";
        return "";
    }

    bool save() {
        File f = LittleFS.open(PATH, "w");
        if (!f) return false;
        JsonDocument d;
        toJson(d.to<JsonObject>(), true);
        const bool ok = serializeJson(d, f) > 0;
        f.close();
        return ok;
    }

    void reset() { Cfg = ObdGwConfig(); save(); }

    void begin() {
        if (!LittleFS.begin(true)) { Serial.println("LittleFS: no se puede montar"); return; }
        File f = LittleFS.open(PATH, "r");
        if (!f) { save(); return; }
        JsonDocument d;
        const DeserializationError e = deserializeJson(d, f);
        f.close();
        if (e) { Serial.printf("config: %s, valores por defecto\n", e.c_str()); return; }
        ObdGwConfig n = Cfg;
        const std::string err = apply(d.as<JsonObjectConst>(), n);
        if (err.empty()) Cfg = n;
        else Serial.printf("config: %s, valores por defecto\n", err.c_str());
    }
}
