cmake_minimum_required(VERSION 3.20)
project(acme_sensor_node VERSION 1.4.2 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(TP ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# --- FreeRTOS-Kernel (vendored, partial copy: CM3 port + heap_4) -------------
add_library(freertos_kernel STATIC
    ${TP}/FreeRTOS-Kernel/tasks.c
    ${TP}/FreeRTOS-Kernel/queue.c
    ${TP}/FreeRTOS-Kernel/list.c
    ${TP}/FreeRTOS-Kernel/timers.c
    ${TP}/FreeRTOS-Kernel/event_groups.c
    ${TP}/FreeRTOS-Kernel/stream_buffer.c
    ${TP}/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
    ${TP}/FreeRTOS-Kernel/portable/MemMang/heap_4.c
)
target_include_directories(freertos_kernel PUBLIC
    ${TP}/FreeRTOS-Kernel/include
    ${TP}/FreeRTOS-Kernel/portable/GCC/ARM_CM3
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)

# --- lwIP (vendored, IPv4 core + netconn API + FreeRTOS sys_arch) ------------
file(GLOB LWIP_SRCS
    ${TP}/lwip/src/core/*.c
    ${TP}/lwip/src/core/ipv4/*.c
    ${TP}/lwip/src/api/*.c
)
add_library(lwip STATIC ${LWIP_SRCS} ${TP}/lwip/contrib/ports/freertos/sys_arch.c)
target_include_directories(lwip PUBLIC
    ${TP}/lwip/src/include
    ${TP}/lwip/contrib/ports/freertos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)
target_link_libraries(lwip PUBLIC freertos_kernel)

# --- mbedTLS (vendored, include/ + library/ only) ----------------------------
file(GLOB MBEDTLS_SRCS ${TP}/mbedtls/library/*.c)
add_library(mbedtls STATIC ${MBEDTLS_SRCS})
target_include_directories(mbedtls PUBLIC
    ${TP}/mbedtls/include
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)
target_include_directories(mbedtls PRIVATE ${TP}/mbedtls/library)
target_compile_definitions(mbedtls PUBLIC MBEDTLS_CONFIG_FILE="acme_mbedtls_config.h")

# --- cJSON (vendored, ACME-patched) ------------------------------------------
add_library(cjson STATIC ${TP}/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${TP}/cJSON)

# --- TinyCBOR (vendored as "cbor", encoder subset) ---------------------------
add_library(cbor STATIC
    ${TP}/cbor/cborencoder.c
    ${TP}/cbor/cborencoder_close_container_checked.c
    ${TP}/cbor/cborerrorstrings.c
)
target_include_directories(cbor PUBLIC ${TP}/cbor)

# --- Application -------------------------------------------------------------
add_executable(sensor-node.elf
    board/startup_mps2_an385.c
    board/uart.c
    board/syscalls.c
    app/main.c
    app/sensor.c
    app/telemetry.c
    app/net.c
)
target_include_directories(sensor-node.elf PRIVATE app board)
target_compile_definitions(sensor-node.elf PRIVATE
    ACME_FW_VERSION="${PROJECT_VERSION}"
)
target_compile_options(sensor-node.elf PRIVATE -Wall -Wextra)
target_link_libraries(sensor-node.elf PRIVATE freertos_kernel lwip mbedtls cjson cbor)
target_link_options(sensor-node.elf PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/board/mps2_an385.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/sensor-node.map
)

add_custom_command(TARGET sensor-node.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary sensor-node.elf sensor-node.bin
    COMMAND ${CMAKE_SIZE} sensor-node.elf
)
