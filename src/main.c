#include <time.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wasm3.h"
#include "extra/fib32.wasm.h"

#define BLINK_SPEED_MS 1000

// temporary test func
uint32_t fib(uint32_t n) {
    if (n <= 1) {
        return n;
    }
    else {
        return fib(n-1) + fib(n - 2);
    }
}

bool status_callback(repeating_timer_t *timer);
volatile uint8_t led = 0b1;

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("CYW43 initialization failed.\n");
        return 1;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);

    repeating_timer_t status_timer;
    add_repeating_timer_ms(-BLINK_SPEED_MS, status_callback, NULL, &status_timer);

    // INFO: WASM code here
    // ---------------------------------------------------------------------------------
    M3Result result = m3Err_none;

    // Calculates and returns a fibonacci number
    uint8_t *wasm = (uint8_t *)fib32_wasm;
    uint32_t size = fib32_wasm_len;

    printf("Loading Webassembly.\n");

    IM3Environment env = m3_NewEnvironment();
    if (!env) {
        printf("m3_NewEnvironment failed.");
        return 1;
    }

    IM3Runtime runtime = m3_NewRuntime(env, 1024, NULL);
    if (!runtime) {
        printf("m3_NewRuntime failed.");
        return 1;
    }

    IM3Module module;
    result = m3_ParseModule(env, &module, wasm, size);
    if (result) {
        printf("m3_ParseModule: %s", result);
        return 1;
    }

    result = m3_LoadModule(runtime, module);
    if (result) {
        printf("m3_LoadModule: %s", result);
        return 1;
    }

    IM3Function func;
    result = m3_FindFunction(&func, runtime, "fib");
    if (result) {
        printf("m3_FindFunction: %s", result);
        return 1;
    }

    printf("Running WebAssembly!\n");

    uint32_t start;
    uint32_t stop;

    // TEST: WASM execution benchmark
    // ---------------------------------------------------------------------------------
    start = to_ms_since_boot(get_absolute_time());
    result = m3_CallV(func, 24);
    if (result) {
        printf("m3_CallV: %s", result);
        return 1;
    }
    stop = to_ms_since_boot(get_absolute_time());
    // TEST: end of benchmark
    // ---------------------------------------------------------------------------------
    uint32_t time_ms_wasm_fib = stop - start;

    // TEST: Native execution benchmark
    // ---------------------------------------------------------------------------------
    start = to_ms_since_boot(get_absolute_time());
    uint32_t native_ret = fib(24);
    stop = to_ms_since_boot(get_absolute_time());
    // TEST: end of benchmark
    // ---------------------------------------------------------------------------------
    uint32_t time_ms_native_fib = stop - start;

    uint32_t ret = 0;
    result = m3_GetResultsV(func, &ret);
    if (result) {
        printf("m3_GetResultsV: %s", result);
        return 1;
    }

    // INFO: end of WASM code
    // ---------------------------------------------------------------------------------

    while (true) {
        sleep_ms(500);
        printf(
            "WebAssembly returned; %u\nExecution took %d ms.\n\n",
            ret, time_ms_wasm_fib
        );
        printf(
            "Native returned; %u\n Execution took %d ms.\n\n",
            native_ret, time_ms_native_fib
        );
    }
    // busy wait
}

bool status_callback(repeating_timer_t *timer) {
    led = !led;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
    return true;
    // keep repeating
}
