# APRS LoRa+ OBD-II — Guía rápida

1. **Monta**: XIAO ESP32-S3 sobre el kit CAN (ese es el bus del **coche**) y un **transceptor CAN externo** en el TWAI nativo
   (TX D2, RX D3) para el bus **NMEA 2000**. Detalle y esquema en [HARDWARE.md](HARDWARE.md).
2. **Instala** el firmware: `pio run -e xiao_esp32s3_obd2 -t upload`, o el `firmware.bin` por el instalador web.
3. **Coche (OBD-II)**: CAN-H al pin 6 del conector OBD-II, CAN-L al pin 14, GND al pin 5. Alimenta el XIAO con un convertidor
   de 12 → 5 V desde el pin 16 (o del encendedor). **No** cierres el puente de 120 Ω del kit.
4. **Bus NMEA 2000**: CAN-H al blanco (pin 4), CAN-L al azul (pin 5), GND al negro (pin 3). El bus necesita sus **dos
   terminadores**; la pasarela no pone ninguno.
5. **WiFi**: conéctate a **APRS-OBD2** (clave `aprsobd2`) y abre `http://192.168.4.1`. Para guardar cambios o actualizar:
   usuario `admin`, clave `123456` (el PIN del ecosistema; cámbialo o déjalo vacío en *Configuración*).
6. **Cristal del MCP2515**: mira lo que pone el cristal del kit (16 u 8 MHz) y ponlo en *Configuración* si no es 16.
7. **Arranca el coche**: en la WebUI, *Bus OBD-II* debe decir **ECU responde = sí** y verás el régimen, la temperatura, etc.
8. **Plotter**: en el plotter APRS LoRa+ o el Garmin, mira los **indicadores de motor** (régimen, temperaturas, combustible,
   batería). Salen de los PGN estándar 127488/127489/127505/127508.

> Solo lee el coche (consultas OBD-II estándar); nunca escribe en su bus.

Manual completo: [MANUAL_ES.md](MANUAL_ES.md).
