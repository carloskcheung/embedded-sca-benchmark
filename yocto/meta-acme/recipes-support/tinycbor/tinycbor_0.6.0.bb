SUMMARY = "Concise Binary Object Representation (CBOR) library"
HOMEPAGE = "https://github.com/intel/tinycbor"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=6c1ac30774dd6476b42b5b020cd2aa5f"

SRC_URI = "git://github.com/intel/tinycbor.git;protocol=https;nobranch=1"
# v0.6.0
SRCREV = "d393c16f3eb30d0c47e6f9d92db62272f0ec4dc7"
S = "${WORKDIR}/git"

# The upstream Makefile probes `file /bin/sh` to decide whether to build a
# shared library; be explicit instead.
EXTRA_OEMAKE += " \
    prefix=${prefix} \
    libdir=${libdir} \
    bindir=${bindir} \
    BUILD_SHARED=1 \
    BUILD_STATIC=1 \
"

do_install() {
    oe_runmake install DESTDIR=${D}
}
