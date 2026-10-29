# Toolchain for the FreeRTOS sensor node: GNU Arm Embedded + CMake + QEMU.
FROM debian:bookworm-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        cmake ninja-build make \
        gcc-arm-none-eabi libnewlib-arm-none-eabi binutils-arm-none-eabi \
        qemu-system-arm \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work
