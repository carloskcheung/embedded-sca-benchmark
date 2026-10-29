FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://acme-mosquitto.conf"

# The broker only serves on-device clients; the cloud uplink is a bridge.
do_install:append() {
    install -m 0644 ${WORKDIR}/acme-mosquitto.conf ${D}${sysconfdir}/mosquitto/mosquitto.conf
}
