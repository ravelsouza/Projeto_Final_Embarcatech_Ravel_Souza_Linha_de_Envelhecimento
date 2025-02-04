#include "functions.h"

// Definições de Variáveis
const float conversion_factor = 3.3f / (1 << 12);
const uint32_t pinos[10] = {4, 8, 9, 16, 17, 18, 19, 20, 21, 22};
const uint32_t pinos_config[10] = {0,0,0,0,0,0,0,0,1,1};

float tensao;
float corrente;

uint32_t start_time;
uint8_t minutos = 0;
uint32_t simulacao[8] = {};
uint32_t quantidade[8] = {};
uint32_t luminarias_acesas = 0;
uint32_t current_time;
uint32_t tempo_de_teste;
uint16_t result;

uint sm;
uint offset;

PIO pio;

bool success;

int main() {
    stdio_init_all();
    start_time = time_us_32();
    inicializa_oled();
    
    
    for(int cont = 0; cont < 10; cont++)
    {
        gpio_init(pinos[cont]);
        gpio_set_dir(pinos[cont], pinos_config[cont]);
        sleep_ms(1);
    }
    for(int i = 5; i < 7; i++)
    {
    gpio_init(i);
    gpio_set_dir(i, GPIO_IN);
    gpio_pull_up(i);
    }

    adc_init();
    adc_gpio_init(PINO_ADC);
    adc_select_input(2);

    init_buzzer(BUZZER_A);
    init_buzzer(BUZZER_B);
    

    // Inicialização do PIO
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Definição das cores
    uint32_t cores[4] = {
        urgb_u32(20, 0, 0),  // Vermelho
        urgb_u32(0, 0, 20),  // Azul 
        urgb_u32(0, 20, 0),  // Verde 
        urgb_u32(20, 20, 20) // Branco 
    };
    while (1) {
        if(!gpio_get(BUTTON_B))
        {
        inicializa_bloco_expositor(pio, sm);

        while(1)
        {
        luminarias_acesas = 0;
        for(int cont = 0; cont < 8; cont ++)
        {
        if(gpio_get(pinos[cont]))
        {
            quantidade[cont] = 1;
            luminarias_acesas += 1;
            simulacao[cont] = urgb_u32(0,5,0);
        }
        else
        {
            quantidade[cont] = 0;
            simulacao[cont] = urgb_u32(5,0,0);
        }
        }
        
        teste_esteira(pio, sm, 25, inicial[7], 7, simulacao);
        result = adc_read();
        tensao = result * conversion_factor;
        corrente = (tensao / 33)* 1000;
        current_time = time_us_32();
        tempo_de_teste = (current_time - start_time)/1000000;
        char valor[100];
        printf("%d %d %d ", quantidade[0], quantidade[1], quantidade[2]);
        printf("%d %d %d ", quantidade[3], quantidade[4], quantidade[5]);
        printf("%d %d ", quantidade[6], quantidade[7]);
        printf("%d %d ", luminarias_acesas, 8 - luminarias_acesas);
        printf("%d:%u ", minutos, tempo_de_teste);
        printf("%0.3f %0.3f\n", tensao, corrente);
        
        if(tempo_de_teste >= 59)
        {
            minutos += 1;
            start_time = time_us_32();
        }
        if(minutos == 6)
        { 
            while(gpio_get(BUTTON_A))
            {
            buzzer_alerta();
            }
            
        } 
        sleep_ms(1000);
        }
        }
    }
}
