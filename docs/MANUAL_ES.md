# APRS LoRa+ OBD-II — Manual de usuario

Copyright (C) 2026 Manuel - EA7JRS · GPL-3.0-or-later · versión 5.4.7 · parte del ecosistema **APRS LoRa+**.

## 1. Qué es

Pasarela **OBD-II → NMEA 2000**. Lee el motor del coche por el puerto de diagnóstico (OBD-II) y publica los datos en un bus
NMEA 2000, para que se vean como **indicadores de motor** tanto en el **plotter APRS LoRa+** como en un **plotter Garmin** (o
cualquier equipo NMEA 2000). Al usar los PGN de motor estándar, ningún plotter necesita código especial.

La placa es un **Seeed XIAO ESP32-S3** con el **kit CAN** (MCP2515) para el coche y un **transceptor CAN externo** para el bus
NMEA 2000. Son dos buses distintos, a 500 y 250 kbit/s. Ver [HARDWARE.md](HARDWARE.md).

## 2. Lo que lee del coche

Consultas OBD-II estándar (ISO 15765-4 / SAE J1979, **modo 01**, solo lectura). Por turnos pide estos PID y los que la ECU
no conteste salen como «sin dato»:

| Dato | PID | Unidad |
|---|---|---|
| Régimen del motor | 0x0C | rpm |
| Velocidad | 0x0D | km/h |
| Temperatura del refrigerante | 0x05 | °C |
| Temperatura del aceite | 0x5C | °C |
| Temperatura de admisión | 0x0F | °C |
| Carga del motor | 0x04 | % |
| Nivel de combustible | 0x2F | % |
| Consumo instantáneo | 0x5E | L/h |
| Tensión del módulo de control | 0x42 | V |
| Presión del colector (MAP) | 0x0B | kPa |
| Tiempo de marcha | 0x1F | s |

No todos los coches dan todos los PID; la pasarela emite solo los que recibe.

## 3. Lo que emite al bus NMEA 2000

| PGN | Qué lleva | Cada |
|---|---|---|
| **127488** Engine Parameters, Rapid | régimen y presión de sobrealimentación | 100 ms |
| **127489** Engine Parameters, Dynamic | refrigerante, aceite, consumo, tensión del alternador, horas, carga | 500 ms |
| **127505** Fluid Level | nivel de combustible (tipo 0) y capacidad del depósito | 2 s |
| **127508** Battery Status | tensión del módulo de control como tensión de batería | 1 s |

Correspondencias: el refrigerante y el aceite pasan a kelvin; la tensión del módulo va como *AltVoltage* (127489) y como
tensión de batería (127508); si *MAP como sobrealimentación* está activado, el 127488 lleva la presión manométrica
(MAP − 101,325 kPa, mínimo 0). La capacidad del depósito solo se emite si la pones en *Configuración*.

La pasarela **reclama una dirección** en el bus (ISO 11783-5) y la defiende; la dirección que queda se guarda y se reutiliza
al arrancar. No es un nodo certificado.

## 4. WebUI

Conéctate por WiFi al punto de acceso **APRS-OBD2** (clave `aprsobd2`) y abre `http://192.168.4.1`. Tres cuadros:

- **Motor (OBD-II)**: régimen, velocidad, temperaturas, carga, combustible, consumo, tensión, presión del colector y tiempo
  de marcha, según lo que conteste la ECU.
- **Bus OBD-II**: si el MCP2515 arrancó, si la ECU responde, y los contadores de consultas, respuestas y errores.
- **NMEA 2000**: si el controlador va, la dirección reclamada (o «perdida»), si está reclamada, y las tramas enviadas y
  recibidas.

Para **guardar** la configuración o **actualizar** el firmware hace falta la clave (usuario `admin`, clave `123456` por
defecto; es el PIN del ecosistema APRS LoRa+, se cambia o se deja vacío en *Configuración*).

## 5. Configuración

| Ajuste | Qué es |
|---|---|
| Nombre / punto de acceso | nombre del punto de acceso, del mDNS y de la red |
| Clave de la WebUI | PIN para guardar y actualizar (vacío = sin clave) |
| WiFi | punto de acceso, cliente (con punto de acceso de rescate) o apagada |
| MCP2515 CS, cristal | pin de selección (D7 = 44) y cristal del kit (16 u 8 MHz) |
| Consulta OBD-II (ms) | tiempo entre peticiones (200 por defecto) |
| Caducidad del dato (ms) | si un dato no llega en este tiempo, se marca «sin dato» (1500 por defecto) |
| NMEA 2000 TX / RX | pines del transceptor externo (D2 = 3 / D3 = 4) |
| Dirección NMEA 2000 | dirección preferida en el bus (se negocia) |
| Instancia del motor | número del motor en los PGN (0 = babor / único) |
| MAP como sobrealimentación | emite el MAP como presión de turbo manométrica en 127488 |
| Capacidad del depósito (L) | para el 127505 (0 = sin dato) |

## 6. Seguridad

- **Solo lectura**: la pasarela solo manda peticiones OBD-II estándar (modo 01) al identificador `0x7DF`. **Nunca** escribe
  tramas en el bus del coche.
- El pin 16 del OBD-II da 12 V aunque el coche esté apagado: no dejes la pasarela enchufada o consumirá batería.
- No une las masas del coche y del barco más que en un punto (la del convertidor).

## 7. Problemas

- **La ECU no responde**: comprueba el cristal del MCP2515 (16 u 8 MHz), que el coche esté en contacto y las conexiones
  CAN-H / CAN-L. Algunos coches necesitan el motor en marcha para dar ciertos PID.
- **El plotter no muestra el motor**: comprueba que *NMEA 2000 → Reclamada = sí* y que el bus tiene sus dos terminadores de
  120 Ω. En el plotter, activa la página o los indicadores de motor y, si pide instancia, la 0.
- **No hay bus NMEA 2000**: revisa el transceptor externo (alimentado a 3V3, CANH/CANL al bus) y los pines TX/RX.

## 8. Actualizar el firmware

Por la WebUI, en *Actualizar el firmware* (`/update`) con la clave; por USB con el [instalador web](https://ea7jrs.github.io/APRS-LoRa-Plus/install/#nautica) (grupo **Náutica**, placa `xiao_esp32s3_obd2`; conserva los ajustes), o compilando con `pio run -e xiao_esp32s3_obd2 -t upload`.
Por USB, la placa contesta a `APRS-INFO?` con `device=obd2`.

## 9. Motor de barco (J1939)

En *Configuración > Motor* elige **Barco: J1939** para un motor diésel marino o industrial con SAE J1939 (CAN de 29 bits, 250 kbit/s; algunos a 500). En ese modo la pasarela **solo escucha**: no envía consultas. Conecta CAN-H / CAN-L / GND de la borna del kit al bus J1939 del motor (conector de diagnóstico Deutsch de 9 pines: C = CAN-H, D = CAN-L, A = masa; o el que indique el fabricante). No cierres el puente de 120 Ω si el bus ya tiene sus terminadores.

| Dato | PGN J1939 | Sale en |
|---|---|---|
| Régimen | 61444 (EEC1) | 127488 |
| Sobrealimentación, temperatura de admisión | 65270 | 127488 |
| Carga | 61443 (EEC2) | 127489 |
| Refrigerante, aceite (temperatura) | 65262 | 127489 |
| Aceite, refrigerante, combustible (presión) | 65263 | 127489 |
| Consumo | 65266 | 127489 |
| Horas totales | 65253 | 127489 |
| Tensión de batería | 65271 | 127489 y 127508 |
| Nivel de combustible | 65276 | 127505 |

Las horas totales muchos motores solo las dan bajo petición; como la pasarela no pide nada, saldrán solo si el motor las difunde. Sin probar con un motor real.

## 10. Telemetría APRS (pendiente)

Está previsto poder mandar la telemetría del motor por APRS al tracker y a la app; de momento solo se guarda la preferencia.

## 11. Formatos

Los formatos de los PGN (NMEA 2000 y J1939) son los de la base de datos de [CANboat](https://canboat.github.io/canboat/) v9.2.0 (Apache-2.0); `tools/canboat_run.sh` los comprueba.
