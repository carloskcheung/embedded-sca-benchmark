cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(acme_ble_controller VERSION 2.1.0)

set(NANOPB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/nanopb)

target_sources(app PRIVATE
    src/main.c
    src/ble.c
    src/motor.c
    src/storage.c
    src/generated/motor.pb.c
    ${NANOPB_DIR}/pb_common.c
    ${NANOPB_DIR}/pb_encode.c
    ${NANOPB_DIR}/pb_decode.c
)
target_include_directories(app PRIVATE src src/generated ${NANOPB_DIR})
target_compile_definitions(app PRIVATE ACME_FW_VERSION="${PROJECT_VERSION}")
