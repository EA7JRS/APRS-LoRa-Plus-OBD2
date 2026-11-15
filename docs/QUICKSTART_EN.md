# APRS LoRa+ OBD-II — Quick start

1. **Assemble**: XIAO ESP32-S3 on the CAN kit (that is the **car** bus) and an **external CAN transceiver** on the native
   TWAI (TX D2, RX D3) for the **NMEA 2000** bus. Details and diagram in [HARDWARE.md](HARDWARE.md).
2. **Flash** the firmware: `pio run -e xiao_esp32s3_obd2 -t upload`, or with the [web installer](https://ea7jrs.github.io/APRS-LoRa-Plus/install/#nautica) (**Nautical** group, board `xiao_esp32s3_obd2`).
3. **Car (OBD-II)**: CAN-H to OBD-II pin 6, CAN-L to pin 14, GND to pin 5. Power the XIAO through a 12 → 5 V converter from
   pin 16 (or the cigarette socket). **Do not** close the kit's 120 Ω jumper.
4. **NMEA 2000 bus**: CAN-H to white (pin 4), CAN-L to blue (pin 5), GND to black (pin 3). The bus needs its **two
   terminators**; the gateway adds none.
5. **WiFi**: join **APRS-OBD2** (password `aprsobd2`) and open `http://192.168.4.1`. To save changes or update: user `admin`,
   password `123456` (the ecosystem PIN; change it or leave it empty in *Configuration*).
6. **MCP2515 crystal**: check the kit's crystal (16 or 8 MHz) and set it in *Configuration* if it is not 16.
7. **Start the car**: in the WebUI, *OBD-II bus* should read **ECU responds = yes** and you will see RPM, temperatures, etc.
8. **Plotter**: on the APRS LoRa+ plotter or the Garmin, look at the **engine instruments** (RPM, temperatures, fuel,
   battery). They come from standard PGNs 127488/127489/127505/127508.

> It only reads the car (standard OBD-II requests); it never writes to its bus.

Full manual: [MANUAL_EN.md](MANUAL_EN.md).
