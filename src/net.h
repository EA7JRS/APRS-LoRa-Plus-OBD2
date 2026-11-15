/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
#pragma once
#include <Arduino.h>
#include <string>

// WiFi (punto de acceso o cliente con punto de acceso de rescate), mDNS, WebUI (puerto 80) y actualización por /update.
namespace Net {
    void        begin();
    void        loop();
    std::string ip();
    bool        apMode();
}
