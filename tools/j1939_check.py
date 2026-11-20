#!/usr/bin/env python3
# Copyright (C) 2026 Manuel - EA7JRS — SPDX-License-Identifier: GPL-3.0-or-later
"""Decodificador J1939 del núcleo contra CANboat: arma cada trama con las definiciones de canboat-j1939.json y comprueba
que lib/ObdCore/src/j1939.h lee los mismos valores. Uso: j1939_check.py <canboat-j1939.json> <dump_frames>"""
import json
import subprocess
import sys

sys.path.insert(0, __import__("os").path.dirname(__file__))
import canboat_check as cb

K = 273.15
# PGN, {campo de CANboat: valor en su unidad (K, Pa, %, rpm, V, L/h, s)}, {clave nuestra: valor esperado}
CASES = [
    (61444, {"Engine RPM": 1812.5}, {"rpm": 1812.5}),
    (61443, {"Engine Percent Load At Current Speed": 47}, {"load": 47}),
    (65262, {"Engine Coolant Temp": 84 + K, "Engine Oil Temp 1": 96.5 + K}, {"coolantC": 84, "oilTempC": 96.5}),
    (65263, {"Engine Fuel Delivery Pressure": 300000, "Engine Oil Pressure": 420000, "Engine Coolant Pressure": 98000},
     {"fuelPressKpa": 300, "oilPressKpa": 420, "coolantPressKpa": 98}),
    (65266, {"Fuel Rate": 11.35}, {"fuelRateLh": 11.35}),
    (65270, {"Boost Pressure": 124000, "Intake Manifold Temp": 41 + K}, {"boostKpa": 124, "intakeC": 41}),
    (65271, {"Battery Voltage": 27.65}, {"volt": 27.65}),
    (65276, {"Fuel Level 1": 62.4}, {"fuelPct": 62.4}),
    (65253, {"Engine Total Hours of Operation": 3600 * 1520}, {"hoursS": 3600 * 1520}),
]


def main(defs, dump):
    version, by = cb.load(defs)
    n = bad = 0
    for pgn, fields, want in CASES:
        p = by[pgn]
        data = bytearray([0xFF] * 8)
        for name, value in fields.items():
            f = cb.field(p, name)
            raw = round((value - (f.get("Offset") or 0)) / (f.get("Resolution") or 1))
            cb.set_bits(data, f["BitOffset"], f["BitLength"], raw)
        out = json.loads(subprocess.check_output([dump, "j1939", str(pgn), data.hex()]))
        n += 1
        if out["pgn"] != pgn:
            print("FALLO %d: no se ha decodificado" % pgn); bad += 1
        for k, v in want.items():
            n += 1
            got = out["v"].get(k)
            if got is None or abs(got - v) > max(abs(v) * 1e-4, 0.02):
                print("FALLO %d %s: leído %r, CANboat %r" % (pgn, k, got, v)); bad += 1
        extra = set(out["v"]) - set(want)
        n += 1
        if extra:
            print("FALLO %d: campos de más con el resto a unos: %s" % (pgn, sorted(extra))); bad += 1
    print("CANboat J1939 %s: %d comprobaciones, %d fallos" % (version, n, bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2]))
