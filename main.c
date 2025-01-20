/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

/**
 * NOTE:
 *  Take into consideration if your WS2812 is a RGB or RGBW variant.
 *
 *  If it is RGBW, you need to set IS_RGBW to true and provide 4 bytes per 
 *  pixel (Red, Green, Blue, White) and use urgbw_u32().
 *
 *  If it is RGB, set IS_RGBW to false and provide 3 bytes per pixel (Red,
 *  Green, Blue) and use urgb_u32().
 *
 *  When RGBW is used with urgb_u32(), the White channel will be ignored (off).
 *
 */
#define IS_RGBW false
#define NUM_PIXELS 25

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 7
#endif

// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}

static inline void acender_leds(PIO pio, uint sm, uint len, int leds[], int row) {
    // Apaga todos os LEDs
    for (uint i = 0; i < len; ++i) {
        bool led_aceso = false;
        // Verifica se o LED atual deve ser aceso (comparando com a linha do vetor de LEDs)
        for (int j = 0; j <= row; ++j) {
            if (leds[j] == i) {
                put_pixel(pio, sm, urgb_u32(0x03, 0, 0)); // Acende o LED (exemplo: vermelho)
                led_aceso = true;
                break;
            }
        }
        if (!led_aceso) {
            put_pixel(pio, sm, urgb_u32(0, 0, 0)); // Apaga os LEDs não escolhidos
        }
    }
}


int main() {
    stdio_init_all();
    printf("WS2812 Smoke Test, usando pino %d\n", WS2812_PIN);

    PIO pio;
    uint sm;
    uint offset;

    // Inicialização do PIO
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Definição da matriz de LEDs
    int inicial[8][8] = {
        {6},
        {6, 7},
        {6, 7, 8},
        {6, 7, 8, 10},
        {6, 7, 8, 10, 18},
        {6, 7, 8, 10, 18, 17},
        {6, 7, 8, 10, 18, 17, 16},
        {6, 7, 8, 10, 18, 17, 16, 14}
    };

    int final[7][7] = {
        {14},
        {16, 14},
        {17, 16, 14},
        {18, 17, 16, 14},
        {10, 18, 17, 16,14},
        {8, 10, 18, 17, 16, 14},
        {7, 8, 10, 18, 17, 16, 14}
    };
    while (1) {
        for (int row = 0; row < 8; ++row) {
            acender_leds(pio, sm, 25, inicial[row], row);
            sleep_ms(200); // Espera 500ms entre as linhas
        }
        for (int row = 6; row >= 0; --row) {
            acender_leds(pio, sm, 25, final[row], row);
            sleep_ms(200); // Espera 500ms entre as linhas
        }
    }

    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
