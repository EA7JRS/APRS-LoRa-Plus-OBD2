/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */

#include "net.h"
#include "config.h"
#include "obd_link.h"
#include "n2k.h"
#include "web_page.h"
#include "board.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <cmath>

namespace Net {

    static const char* USER = "admin";
    static WebServer web(80);
    static HTTPUpdateServer updater;
    static bool ap = false, started = false;
    static uint32_t connectFrom = 0, rebootAt = 0;

    bool apMode() { return ap; }
    std::string ip() { return (ap ? WiFi.softAPIP() : WiFi.localIP()).toString().c_str(); }

    static bool allowed() {
        if (Cfg.pin.empty() || web.authenticate(USER, Cfg.pin.c_str())) return true;
        web.requestAuthentication();
        return false;
    }

    static void addNum(JsonObject o, const char* k, float v) { if (!std::isnan(v)) o[k] = v; }

    static void status() {
        JsonDocument d;
        d["device"] = "obd2";
        d["board"] = OBD2_BOARD;
        d["version"] = OBD2_VERSION;
        d["name"] = Cfg.name;

        JsonObject w = d["wifi"].to<JsonObject>();
        w["mode"] = Cfg.wifiMode;
        w["ap"] = ap;
        w["ip"] = ip();
        if (!ap && WiFi.status() == WL_CONNECTED) w["rssi"] = WiFi.RSSI();
        if (WiFi.getMode() & WIFI_MODE_AP) w["clients"] = WiFi.softAPgetStationNum();

        const Obd::Data o = ObdLink::snapshot();
        JsonObject b = d["obd"].to<JsonObject>();
        b["proto"] = Cfg.proto;
        b["bus"] = ObdLink::busOk();
        b["ecu"] = ObdLink::ecuAlive();
        b["requests"] = ObdLink::requests();
        b["responses"] = ObdLink::responses();
        b["errors"] = ObdLink::rxErrors();
        addNum(b, "rpm", o.rpm);
        addNum(b, "speed", o.speedKmh);
        addNum(b, "coolant", o.coolantC);
        addNum(b, "oil", o.oilTempC);
        addNum(b, "intake", o.intakeC);
        addNum(b, "load", o.loadPct);
        addNum(b, "fuel", o.fuelPct);
        addNum(b, "volt", o.moduleV);
        addNum(b, "fuelRate", o.fuelRateLh);
        addNum(b, "map", o.mapKpa);
        addNum(b, "oilP", o.oilPressKpa);
        addNum(b, "coolP", o.coolantPressKpa);
        addNum(b, "fuelP", o.fuelPressKpa);
        addNum(b, "boost", o.boostKpa);
        if (o.engineHoursS != 0xFFFFFFFF) b["hours"] = o.engineHoursS / 3600.0;
        if (o.runtimeS != 0xFFFFFFFF) b["runtime"] = o.runtimeS;

        const N2k::Status& n = N2k::status();
        JsonObject k = d["n2k"].to<JsonObject>();
        k["running"] = n.running;
        k["address"] = n.address;
        k["claimed"] = n.claimed;
        k["lost"] = n.lost;
        k["sent"] = n.sent;
        k["received"] = n.received;
        k["claims"] = n.claims;

        String out;
        serializeJson(d, out);
        web.sendHeader("Cache-Control", "no-store");
        web.send(200, "application/json", out);
    }

    static void getConfig() {
        JsonDocument d;
        Config::toJson(d.to<JsonObject>(), false);
        String out;
        serializeJson(d, out);
        web.send(200, "application/json", out);
    }

    static void postConfig() {
        if (!allowed()) return;
        JsonDocument d;
        if (deserializeJson(d, web.arg("plain"))) { web.send(400, "application/json", "{\"error\":\"json\"}"); return; }
        ObdGwConfig n = Cfg;
        const std::string err = Config::apply(d.as<JsonObjectConst>(), n);
        if (!err.empty()) { web.send(400, "application/json", String("{\"error\":\"") + err.c_str() + "\"}"); return; }
        Cfg = n;
        Config::save();
        web.send(200, "application/json", "{\"ok\":true}");
    }

    static void startServices() {
        if (started) return;
        started = true;
        web.on("/", HTTP_GET, [] { web.sendHeader("Cache-Control", "no-store"); web.send_P(200, "text/html", WEB_PAGE); });
        web.on("/api/status", HTTP_GET, status);
        web.on("/api/config", HTTP_GET, getConfig);
        web.on("/api/config", HTTP_POST, postConfig);
        web.on("/api/reboot", HTTP_POST, [] { if (!allowed()) return; web.send(200, "application/json", "{\"ok\":true}"); rebootAt = millis() + 500; });
        web.on("/api/reset", HTTP_POST, [] { if (!allowed()) return; Config::reset(); web.send(200, "application/json", "{\"ok\":true}"); rebootAt = millis() + 500; });
        updater.setup(&web, "/update", Cfg.pin.empty() ? "" : USER, Cfg.pin.c_str());
        web.begin();
        if (MDNS.begin(Cfg.name.c_str())) MDNS.addService("http", "tcp", 80);
        Serial.printf("[net] WebUI en http://%s/\n", ip().c_str());
    }

    static void startAp() {
        WiFi.mode(Cfg.wifiMode == 1 ? WIFI_AP_STA : WIFI_AP);
        WiFi.softAP(Cfg.name.c_str(), Cfg.apPass.c_str());
        ap = true;
        startServices();
    }

    void begin() {
        WiFi.persistent(false);
        WiFi.setHostname(Cfg.name.c_str());
        if (Cfg.wifiMode == 0) { WiFi.mode(WIFI_OFF); return; }
        if (Cfg.wifiMode == 2 || Cfg.ssid.empty()) { startAp(); return; }
        WiFi.mode(WIFI_STA);
        WiFi.begin(Cfg.ssid.c_str(), Cfg.pass.c_str());
        connectFrom = millis();
    }

    void loop() {
        if (rebootAt && millis() > rebootAt) ESP.restart();
        if (Cfg.wifiMode == 1 && !ap && !started) {
            if (WiFi.status() == WL_CONNECTED) startServices();
            else if (millis() - connectFrom > 30000) startAp();        // sin conexión: punto de acceso de rescate
        }
        if (started) web.handleClient();
    }
}
