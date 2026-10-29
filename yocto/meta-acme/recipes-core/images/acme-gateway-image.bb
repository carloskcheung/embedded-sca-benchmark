SUMMARY = "Acme industrial gateway image"
DESCRIPTION = "Linux image for the Acme gateway: telemetry daemon, local MQTT \
broker and the firmware for the attached sensor-node and BLE controller MCUs."
LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += "ssh-server-openssh"

IMAGE_INSTALL:append = " \
    acme-gatewayd \
    acme-mcu-firmware \
    mosquitto \
    mosquitto-clients \
    sqlite3 \
    libmodbus \
    can-utils \
"

IMAGE_ROOTFS_EXTRA_SPACE = "131072"
