# Acme industrial gateway - top-level build. Every toolchain runs in Docker.
#
#   make sensor-node      FreeRTOS firmware (CMake, vendored third_party/)
#   make run-sensor-node  boot it in QEMU (Ctrl-A X to quit)
#   make ble-controller   Zephyr firmware (west manifest)
#   make gatewayd         build + smoke-test acme-gatewayd on Debian
#   make yocto-parse      fetch layers + parse all recipes (quick sanity check)
#   make yocto            full Yocto image incl. SPDX SBOM (hours, ~100 GB)
#   make sbom             copy the Yocto SPDX output into benchmark/sbom/

ROOT        := $(abspath .)
DOCKER_RUN  := docker run --rm
FW_OUT      := $(ROOT)/build/firmware

SENSOR_IMG  := acme/sensor-node-build
ZEPHYR_IMG  := acme/zephyr-build
GATEWAY_IMG := acme/gatewayd-build
KAS_IMG     := ghcr.io/siemens/kas/kas:4.5

# Yocto must not build on the case-insensitive macOS filesystem, so its work
# tree, downloads and sstate live in a Docker volume.
YOCTO_VOL   := acme-yocto-work
ZEPHYR_VOL  := acme-zephyr-ws

.PHONY: all firmware sensor-node run-sensor-node ble-controller gatewayd stage-firmware yocto-volume yocto-parse yocto sbom clean

all: firmware gatewayd

firmware: sensor-node ble-controller

# --- FreeRTOS sensor node ----------------------------------------------------
sensor-node:
	docker build -q -t $(SENSOR_IMG) -f docker/sensor-node.Dockerfile docker
	$(DOCKER_RUN) -v $(ROOT)/firmware/sensor-node:/work $(SENSOR_IMG) sh -c '\
	    cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
	          -DCMAKE_BUILD_TYPE=MinSizeRel && cmake --build build'
	mkdir -p $(FW_OUT)
	cp firmware/sensor-node/build/sensor-node.elf firmware/sensor-node/build/sensor-node.bin $(FW_OUT)/

run-sensor-node: sensor-node
	$(DOCKER_RUN) -it -v $(ROOT)/firmware/sensor-node:/work $(SENSOR_IMG) \
	    qemu-system-arm -M mps2-an385 -nographic -kernel build/sensor-node.elf

# --- Zephyr BLE controller ---------------------------------------------------
ble-controller:
	docker build -q -t $(ZEPHYR_IMG) -f docker/zephyr.Dockerfile docker
	$(DOCKER_RUN) -v $(ZEPHYR_VOL):/ws -v $(ROOT)/firmware/ble-controller:/ws/ble-controller \
	    $(ZEPHYR_IMG) bash -c '\
	    set -e; \
	    [ -d /ws/.west ] || west init -l /ws/ble-controller; \
	    west update --narrow -o=--depth=1; \
	    west zephyr-export; \
	    west build -p auto -b nrf52840dk/nrf52840 -d /ws/build/ble-controller /ws/ble-controller/app; \
	    mkdir -p /ws/ble-controller/build && cp /ws/build/ble-controller/zephyr/zephyr.hex \
	        /ws/build/ble-controller/zephyr/zephyr.elf /ws/ble-controller/build/'
	mkdir -p $(FW_OUT)
	cp firmware/ble-controller/build/zephyr.hex $(FW_OUT)/ble-controller.hex
	cp firmware/ble-controller/build/zephyr.elf $(FW_OUT)/ble-controller.elf

# --- Linux gateway daemon (native smoke test outside Yocto) ------------------
gatewayd: sensor-node
	docker build -q -t $(GATEWAY_IMG) -f docker/gatewayd.Dockerfile docker
	$(DOCKER_RUN) -v $(ROOT)/software/acme-gatewayd:/src -v $(ROOT)/firmware/sensor-node:/fw \
	    -v $(ROOT)/docker:/scripts:ro $(GATEWAY_IMG) /scripts/gatewayd-smoke-test.sh

# --- Yocto -------------------------------------------------------------------
KAS_RUN := $(DOCKER_RUN) -v $(YOCTO_VOL):/work -v $(ROOT):/repo:ro \
	-e KAS_WORK_DIR=/work -e KAS_BUILD_DIR=/work/build \
	-e GIT_CONFIG_COUNT=1 -e GIT_CONFIG_KEY_0=safe.directory -e GIT_CONFIG_VALUE_0='*' \
	$(KAS_IMG)

yocto-volume:
	$(DOCKER_RUN) --user root -v $(YOCTO_VOL):/work --entrypoint chown $(KAS_IMG) builder:builder /work

# BitBake checksums file:// sources at parse time, so the firmware blobs must
# be staged into meta-acme before any parse or build.
FW_STAGE := yocto/meta-acme/recipes-firmware/acme-mcu-firmware/files/bin

stage-firmware: firmware
	rm -rf $(FW_STAGE) && mkdir -p $(FW_STAGE)
	cp $(FW_OUT)/sensor-node.bin $(FW_OUT)/ble-controller.hex $(FW_STAGE)/

# Fetch the pinned layers and parse every recipe (minutes, not hours).
yocto-parse: stage-firmware yocto-volume
	$(KAS_RUN) shell /repo/yocto/kas/acme-gateway.yml -c 'bitbake -p && bitbake-layers show-layers && bitbake-layers show-appends | grep -A1 acme'

yocto: stage-firmware yocto-volume
	$(KAS_RUN) build /repo/yocto/kas/acme-gateway.yml

sbom:
	mkdir -p benchmark/sbom
	$(DOCKER_RUN) -v $(YOCTO_VOL):/work -v $(ROOT)/benchmark/sbom:/out $(KAS_IMG) sh -c '\
	    cp /work/build/tmp/deploy/images/qemuarm64/acme-gateway-image-qemuarm64.rootfs.spdx.tar.zst /out/ && \
	    cd /out && rm -rf spdx && mkdir spdx && tar --zstd -xf *.spdx.tar.zst -C spdx'

clean:
	rm -rf build firmware/sensor-node/build firmware/ble-controller/build
