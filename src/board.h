/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// Pines del XIAO ESP32-S3 y versión. Dos buses CAN: el OBD-II del coche por el MCP2515 del kit (SPI) y el NMEA 2000 por el
// TWAI nativo con un transceptor externo. Los pines se pueden fijar con -D en platformio.ini; aquí van los de serie.
//
// XIAO ESP32-S3: D0 = GPIO1, D1 = GPIO2, D2 = GPIO3, D3 = GPIO4, D4 = GPIO5 (SDA), D5 = GPIO6 (SCL),
//                D6 = GPIO43 (TX), D7 = GPIO44 (RX), D8 = GPIO7 (SCK), D9 = GPIO8 (MISO), D10 = GPIO9 (MOSI)
#pragma once

#define OBD2_VERSION "5.4.6"

#ifndef OBD2_BOARD
    #define OBD2_BOARD "xiao_esp32s3_obd2"
#endif

// MCP2515 (OBD-II, 500 kbit/s): CS por D7 (GPIO44); SPI por D8/D9/D10 (los de serie del XIAO). Cristal 16 u 8 MHz (config).
#ifndef OBD2_MCP_CS
    #define OBD2_MCP_CS 44
#endif
// Interrupción del MCP2515: -1 = sin usar (se consulta en el bucle).
#ifndef OBD2_MCP_INT
    #define OBD2_MCP_INT -1
#endif

// TWAI nativo (NMEA 2000, 250 kbit/s) al transceptor externo: dos GPIO libres del XIAO.
#ifndef OBD2_CAN_TX
    #define OBD2_CAN_TX 3           // D2 = GPIO3
#endif
#ifndef OBD2_CAN_RX
    #define OBD2_CAN_RX 4           // D3 = GPIO4
#endif
