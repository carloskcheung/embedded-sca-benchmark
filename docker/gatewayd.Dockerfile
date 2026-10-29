# Native (non-Yocto) build + smoke-test environment for acme-gatewayd.
FROM debian:bookworm-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config git ca-certificates sqlite3 \
        libcjson-dev libmosquitto-dev libsqlite3-dev libssl-dev \
        qemu-system-arm \
 && rm -rf /var/lib/apt/lists/*

# TinyCBOR is not packaged by Debian; build the same release Yocto uses.
RUN git clone --depth 1 -b v0.6.0 https://github.com/intel/tinycbor /tmp/tinycbor \
 && make -C /tmp/tinycbor prefix=/usr/local \
 && make -C /tmp/tinycbor prefix=/usr/local install \
 && ldconfig && rm -rf /tmp/tinycbor
