#!/bin/sh
# Build acme-gatewayd natively and feed it live output from the sensor-node
# firmware running in QEMU. Passes if authenticated frames reach SQLite.
set -eu

cmake -S /src -B /tmp/gw -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build /tmp/gw

DB=/tmp/telemetry.db
printf 'db_path=%s\nmqtt_host=127.0.0.1\n' "$DB" > /tmp/gw.conf

timeout 6 qemu-system-arm -M mps2-an385 -nographic -monitor none \
    -kernel /fw/build/sensor-node.elf 2>/dev/null \
  | /tmp/gw/acme-gatewayd -c /tmp/gw.conf -i - || true

ROWS=$(sqlite3 "$DB" 'SELECT COUNT(*) FROM telemetry')
echo "smoke test: $ROWS verified frames stored"
[ "$ROWS" -ge 3 ]
