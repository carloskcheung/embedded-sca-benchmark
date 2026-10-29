# Toolchain for the Zephyr BLE controller: west + Zephyr SDK (ARM toolchain only).
FROM debian:bookworm-slim

ARG ZEPHYR_SDK_VERSION=0.16.8
ARG TARGETARCH

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
        python3 python3-pip python3-venv python3-dev xz-utils file make \
        curl wget ca-certificates \
 && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv
ENV PATH=/opt/venv/bin:$PATH
# Mirrors zephyr v3.7 scripts/requirements-base.txt (build-time subset).
RUN pip install --no-cache-dir \
        west==1.2.0 pyelftools PyYAML pykwalify packaging progress psutil \
        pyserial requests anytree intelhex jsonschema

RUN set -eux; \
    case "$TARGETARCH" in arm64) HOST=aarch64 ;; amd64) HOST=x86_64 ;; *) exit 1 ;; esac; \
    base=https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}; \
    cd /opt; \
    curl -fsSL $base/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-${HOST}_minimal.tar.xz | tar -xJ; \
    cd zephyr-sdk-${ZEPHYR_SDK_VERSION}; \
    curl -fsSL $base/toolchain_linux-${HOST}_arm-zephyr-eabi.tar.xz | tar -xJ; \
    ./setup.sh -c

ENV ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-${ZEPHYR_SDK_VERSION}
WORKDIR /ws
