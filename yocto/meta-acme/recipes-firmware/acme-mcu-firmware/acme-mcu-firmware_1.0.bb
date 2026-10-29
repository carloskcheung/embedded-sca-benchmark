SUMMARY = "Firmware for the Acme gateway's companion MCUs"
DESCRIPTION = "Prebuilt images for the FreeRTOS sensor node and the Zephyr \
BLE motor controller, flashed by the gateway at first boot."
LICENSE = "CLOSED"

# `make yocto` copies the freshly built binaries here before invoking kas.
SRC_URI = " \
    file://bin/sensor-node.bin \
    file://bin/ble-controller.hex \
"
S = "${WORKDIR}"

inherit allarch

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware/acme
    install -m 0644 ${WORKDIR}/bin/sensor-node.bin ${D}${nonarch_base_libdir}/firmware/acme/
    install -m 0644 ${WORKDIR}/bin/ble-controller.hex ${D}${nonarch_base_libdir}/firmware/acme/
}

FILES:${PN} = "${nonarch_base_libdir}/firmware/acme"
INSANE_SKIP:${PN} += "arch"
