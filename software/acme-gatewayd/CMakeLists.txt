cmake_minimum_required(VERSION 3.16)
project(acme-gatewayd VERSION 3.0.4 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET
    libcjson
    libmosquitto
    sqlite3
    libcrypto
    tinycbor
)

add_executable(acme-gatewayd
    src/main.c
    src/frame.c
    src/store.c
    src/publish.c
)
target_compile_definitions(acme-gatewayd PRIVATE ACME_GATEWAYD_VERSION="${PROJECT_VERSION}")
target_compile_options(acme-gatewayd PRIVATE -Wall -Wextra)
target_link_libraries(acme-gatewayd PRIVATE PkgConfig::DEPS)

include(GNUInstallDirs)
install(TARGETS acme-gatewayd DESTINATION ${CMAKE_INSTALL_SBINDIR})
install(FILES acme-gatewayd.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
