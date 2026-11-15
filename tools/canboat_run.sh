#!/bin/sh
# Copyright (C) 2026 Manuel - EA7JRS — SPDX-License-Identifier: GPL-3.0-or-later
# Contrasta el núcleo con una release FIJADA de CANboat: los codificadores NMEA 2000 (canboat.json) y el decodificador
# J1939 (canboat-j1939.json). Uso: tools/canboat_run.sh [carpeta con los dos json]; sin carpeta, los baja de GitHub.
set -e
cd "$(dirname "$0")/.."
CANBOAT_VERSION=v9.2.0
DIR="${1:-.build/canboat-$CANBOAT_VERSION}"
mkdir -p .build "$DIR"
for f in canboat.json canboat-j1939.json; do
    [ -s "$DIR/$f" ] || curl -fsSL "https://raw.githubusercontent.com/canboat/canboat/$CANBOAT_VERSION/docs/$f" -o "$DIR/$f"
done
g++ -std=gnu++17 -O1 -Wall -Wextra -Werror -Ilib/ObdCore/src test_host/dump_frames.cpp -o .build/dump_frames
./.build/dump_frames | python3 tools/canboat_check.py comprobar "$DIR/canboat.json"
python3 tools/j1939_check.py "$DIR/canboat-j1939.json" ./.build/dump_frames
