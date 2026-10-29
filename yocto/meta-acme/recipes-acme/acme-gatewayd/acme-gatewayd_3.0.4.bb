SUMMARY = "Acme gateway telemetry daemon"
DESCRIPTION = "Ingests authenticated CBOR telemetry from the sensor-node MCU, \
buffers it in SQLite and forwards it to the local MQTT broker."
HOMEPAGE = "https://acme-industrial.example/gateway"
LICENSE = "CLOSED"

# Application sources live in software/ at the top of this repository.
FILESEXTRAPATHS:prepend := "${THISDIR}/../../../../software:"

SRC_URI = " \
    file://acme-gatewayd/ \
    file://acme-gatewayd.service \
"
S = "${WORKDIR}/acme-gatewayd"

DEPENDS = "cjson mosquitto sqlite3 openssl tinycbor"

inherit cmake pkgconfig systemd

SYSTEMD_SERVICE:${PN} = "acme-gatewayd.service"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/acme-gatewayd.service ${D}${systemd_system_unitdir}/
    install -d ${D}${localstatedir}/lib/acme
}

RDEPENDS:${PN} += "mosquitto"
