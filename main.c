#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

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

static inline void acender_leds(PIO pio, uint sm, uint len, int leds[], int row, uint32_t cor) {
    // Apaga todos os LEDs
    for (uint i = 0; i < len; ++i) {
        bool led_aceso = false;
        // Verifica se o LED atual deve ser aceso (comparando com a linha do vetor de LEDs)
        for (int j = 0; j <= row; ++j) {
            if (leds[j] == i) {
                put_pixel(pio, sm, cor); // Acende o LED na cor especificada
                led_aceso = true;
                break;
            }
        }
        if (!led_aceso) {
            put_pixel(pio, sm, urgb_u32(0, 0, 0)); // Apaga os LEDs não escolhidos
        }
    }
}

static inline void teste_esteira(PIO pio, uint sm, uint len, int leds[], int row, uint32_t* cor) {
    // Apaga todos os LEDs
    int cor_led = 0;
    for (uint i = 0; i < len; ++i) {
        bool led_aceso = false;
        // Verifica se o LED atual deve ser aceso (comparando com a linha do vetor de LEDs)
        for (int j = 0; j <= row; ++j) {
            if (leds[j] == i) {
                put_pixel(pio, sm, cor[cor_led]); // Acende o LED na cor especificada
                cor_led += 1;
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
    gpio_init(6);
    gpio_set_dir(6, GPIO_IN);
    gpio_pull_up(6);
    gpio_init(4);
    gpio_set_dir(4, GPIO_IN);
    gpio_init(8);
    gpio_set_dir(8, GPIO_IN);
    gpio_init(9);
    gpio_set_dir(9, GPIO_IN);
    gpio_init(16);
    gpio_set_dir(16, GPIO_IN);
    gpio_init(17);
    gpio_set_dir(17, GPIO_IN);
    gpio_init(18);
    gpio_set_dir(18, GPIO_IN);
    gpio_init(19);
    gpio_set_dir(19, GPIO_IN);
    gpio_init(20);
    gpio_set_dir(20, GPIO_IN);

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
        {10, 18, 17, 16, 14},
        {8, 10, 18, 17, 16, 14},
        {7, 8, 10, 18, 17, 16, 14}
    };

    int vazia[] = {};
    // Definição das cores
    uint32_t cores[4] = {
        urgb_u32(20, 0, 0),    // Vermelho (R, G, B)
        urgb_u32(0, 0, 20),    // Azul (R, G, B)
        urgb_u32(0, 20, 0),    // Verde (R, G, B)
        urgb_u32(20, 20, 20) // Branco (R, G, B)
    };

    int cor_index = 0;

    while (1) {
        if(!gpio_get(6))
        {
        // Executa os loops 4 vezes com cores diferentes
        for (int ciclo = 0; ciclo < 4; ++ciclo) {
            // Loop para a matriz inicial
            for (int row = 0; row < 8; ++row) {
                acender_leds(pio, sm, 25, inicial[row], row, cores[cor_index]);
                sleep_ms(200); // Espera 200ms entre as linhas
            }
            
            // Loop para a matriz final
            for (int row = 6; row >= 0; --row) {
                acender_leds(pio, sm, 25, final[row], row, cores[cor_index]);
                sleep_ms(200); // Espera 200ms entre as linhas
            }

            // Muda a cor para o próximo ciclo
            cor_index = (cor_index + 1) % 4;
        }
        acender_leds(pio, sm, 25, inicial[7], 7, cores[3]);
        sleep_ms(300);
        for(int i = 0; i < 2; i++)
        {
        acender_leds(pio, sm, 25, inicial[7], 7, urgb_u32(0,0,0));
        sleep_ms(300);
        acender_leds(pio, sm, 25, inicial[7], 7, cores[3]);
        sleep_ms(300);
        }
        uint32_t simulacao[8] = {};
        
        while(1)
        {
        if(gpio_get(4))
        {
            simulacao[0] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[0] = urgb_u32(50,0,0);
        }
        if(gpio_get(8))
        {
            simulacao[1] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[1] = urgb_u32(50,0,0);
        }
        if(gpio_get(9))
        {
             simulacao[2] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[2] = urgb_u32(50,0,0);
        }
        if(gpio_get(16))
        {    
            simulacao[3] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[3] = urgb_u32(50,0,0);
        }
        if(gpio_get(17))
        {
              simulacao[4] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[4] = urgb_u32(50,0,0);
        }
        if(gpio_get(18))
        {
             simulacao[5] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[6] = urgb_u32(50,0,0);
        }
        if(gpio_get(19))
        {
             simulacao[6] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[6] = urgb_u32(50,0,0);
        }

        if(gpio_get(20))
        {
              simulacao[7] = urgb_u32(0,50,0);
        }
        else
        {
            simulacao[7] = urgb_u32(50,0,0);
        }
        teste_esteira(pio, sm, 25, inicial[7], 7, simulacao);
        sleep_ms(1000);
        }
        }
    }

    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
