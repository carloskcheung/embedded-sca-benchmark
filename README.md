# Acme Industrial Gateway: embedded SCA benchmark

A realistic embedded product repository for testing how well a scanner
(FOSSA) identifies third-party components across the three ways embedded
teams pull them in:

| Part | Stack | How third-party code gets in |
|---|---|---|
| `firmware/sensor-node` | FreeRTOS 11.1 on Cortex-M3, CMake | **Vendored** into `third_party/`: partial copies, a local patch, one renamed dir with its LICENSE dropped |
| `firmware/ble-controller` | Zephyr 3.7.1 LTS on nRF52840, west | **Manifest**: `west.yml` pins Zephyr + 6 modules by SHA; nanopb vendored in `lib/` |
| `yocto/` | Yocto scarthgap (5.0 LTS), kas, qemuarm64 | **Recipes**: pinned upstream layers, custom `meta-acme` layer with a new recipe, bbappends and a patch; build emits an **SPDX SBOM** |
| `software/acme-gatewayd` | C daemon (first-party) | Links cJSON, mosquitto, SQLite, OpenSSL, TinyCBOR via the Yocto recipe |

The product story: the sensor node samples the environment and emits
HMAC-signed CBOR telemetry. The gateway daemon verifies, stores and publishes
it over MQTT. The BLE controller drives a motor. Both MCU firmware images ship
inside the Linux image as a `LICENSE = "CLOSED"` blob, the way real products
do.

## Ground truth and scoring

[`benchmark/expected-components.yaml`](benchmark/expected-components.yaml)
lists every planted component with exact version, SPDX license, purl and its
`inclusion` type (`vendored-partial`, `vendored-obfuscated`, `west-manifest`,
`yocto-recipe-patched`, `binary-embedded`, …).

```sh
pip install pyyaml
python3 benchmark/score.py <fossa-export.json>                # CycloneDX or SPDX JSON
python3 benchmark/score.py --scope yocto benchmark/sbom/spdx/ # score the Yocto SBOM itself
```

`benchmark/sbom/spdx/` is the SPDX 2.2 output of a full build of this
repository (`make yocto && make sbom`), committed so scanners can read it
without a multi-hour build. Its own score is in
[`benchmark/results-yocto-spdx.md`](benchmark/results-yocto-spdx.md): every
component and version is right, but Yocto leaves four licenses in its own
naming (`LicenseRef-PD`, `LicenseRef-EDL-1.0`, …), which score.py reports as
`yocto-name`.

### Deliberate traps

- **Same component, several identities.** Mbed TLS 3.6.2 appears vendored (sensor
  node) and as Zephyr's fork (BLE controller). cJSON appears as vendored
  1.7.18 and as Yocto 1.7.19. TinyCBOR appears obfuscated in firmware and as a
  clean custom recipe.
- **Modified vendored code.** `third_party/cJSON/cJSON.h` carries an
  `ACME-PATCH` edit.
- **No LICENSE file.** `third_party/cbor/` is TinyCBOR with only per-file
  headers left.
- **Yocto license naming.** `PD` (SQLite) and `EDL-1.0` (Mosquitto) must map to
  SPDX `blessing` and `BSD-3-Clause`.
- **Opaque firmware.** `acme-mcu-firmware` is `CLOSED` in the Yocto SBOM but
  contains 13 open-source components.

## Building

Everything runs in Docker, and the host needs only Docker and `make`.

```sh
make sensor-node       # FreeRTOS firmware           (~1 min)
make run-sensor-node   # boot it in QEMU, prints signed JSON telemetry
make ble-controller    # Zephyr firmware, fetches ~600 MB via west
make gatewayd          # gateway daemon + end-to-end smoke test against QEMU
make yocto-parse       # fetch layers, parse all recipes  (~10 min)
make yocto             # full image + SPDX SBOM           (hours)
make sbom              # copy SPDX output to benchmark/sbom/
```

**Yocto needs room.** Give Docker Desktop at least **150 GB of disk** and
**8 GB+ RAM** (Settings → Resources). The Yocto work tree lives in the
`acme-yocto-work` Docker volume because BitBake cannot build on macOS's
case-insensitive filesystem.

## Pins

| | Version | Pin |
|---|---|---|
| poky | scarthgap | `b6ae714a1d1bfae8384654a13da15996cac64712` |
| meta-openembedded | scarthgap | `2db6aece32c7f8ebf6015b468e1b4e21de072e3b` |
| Zephyr | v3.7.1 | `9f824289b28d7aea2eee74f62787c385a5005453` |
| Zephyr SDK | 0.16.8 | `docker/zephyr.Dockerfile` |
