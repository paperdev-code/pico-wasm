cmake_minimum_required(VERSION 3.21)

include(cmake/pico_sdk_import.cmake)

set(PICO_BOARD "pico_w")

project(pico_wasm3_runtime C CXX ASM)
set(CMAKE_C_STANDARD 17)

pico_sdk_init()

# the wasm3 library
add_subdirectory(lib/wasm3/source)

add_executable(wasm_runtime
    src/main.c
    )

pico_enable_stdio_usb(wasm_runtime TRUE)
pico_enable_stdio_uart(wasm_runtime FALSE)

target_link_libraries(wasm_runtime
    # pico sdk
    pico_stdlib
    pico_cyw43_arch_none
    # wasm3
    m3
    )

# emit .uf2 file
pico_add_extra_outputs(wasm_runtime)
