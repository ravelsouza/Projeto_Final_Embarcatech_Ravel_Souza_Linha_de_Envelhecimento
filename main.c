#include "functions.h"

// Definições de Variáveis
const float fator_de_conversao = 3.3f / (1 << 12);
const uint32_t pinos[10] = {4, 8, 9, 16, 17, 18, 19, 20, 21, 22};
const uint32_t pinos_config[10] = {0,0,0,0,0,0,0,0,1,1};
const uint32_t todos_os_leds[8] = {6, 7, 8, 10, 18, 17, 16, 14};

float tensao;
float corrente;

uint8_t minutos = 0;
uint32_t cor_na_matriz[8] = {};
uint32_t estado_dos_leds[8] = {};
uint32_t luminarias_acesas = 0;
uint32_t tempo_inicial;
uint32_t tempo_atual;
uint32_t tempo_de_teste;
uint16_t leitura;

uint sm;
uint offset;

PIO pio;

bool success;

int main() {

    stdio_init_all();
    tempo_inicial = time_us_32();
    inicializa_oled();
    
    for(int cont = 0; cont < 10; cont++) // Inicializa os pinos
    {
        gpio_init(pinos[cont]);
        gpio_set_dir(pinos[cont], pinos_config[cont]);
        sleep_ms(1);
    }

    for(int i = 5; i < 7; i++) // Inicializa os botões
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
    
    // Inicializa o PIO
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

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
        if(gpio_get(pinos[cont])) // Se houver tensão no pino, o led correspondente na Matriz deve acender na cor verde.
        {
            estado_dos_leds[cont] = 1;
            luminarias_acesas += 1; // Adiciona-se 1 para cada luminária acesa na esteira.
            cor_na_matriz[cont] = urgb_u32(0,5,0); 
        }
        else // Se não houver tensão no pino, o led correspondente deve acender na cor vermelha.
        {
            estado_dos_leds[cont] = 0;
            cor_na_matriz[cont] = urgb_u32(5,0,0); 
        }
        }

        // Recebe uma lista a posição dos leds na Matriz e outra com as suas respectivas cores para setá-las na Matriz.
        teste_esteira(pio, sm, 25, todos_os_leds, 7, cor_na_matriz);

        leitura = adc_read();
        tensao = leitura * fator_de_conversao; 
        corrente = (tensao / 33)* 1000; // Usa o valor da tensão para calcular a corrente. OBS: Resistor --> 33 Ohms
        tempo_atual = time_us_32();
        tempo_de_teste = (tempo_atual - tempo_inicial)/1000000; 

        // Imprime os dados
        for(int controle = 0; controle < 8; controle++)
        {
        printf("%d ", estado_dos_leds[controle]);
        }

        printf("%d %d ", luminarias_acesas, 8 - luminarias_acesas);
        printf("%d:%u ", minutos, tempo_de_teste);
        printf("%0.3f %0.3f\n", tensao, corrente);
        
        if(tempo_de_teste >= 59) // Contagem de minutos
        {
            minutos += 1;
            tempo_inicial = time_us_32();
        }

        while(minutos == 6) // Checagem do tempo Limite (6 minutos)
        { 
            buzzer_alerta();
            if(!gpio_get(BUTTON_A))
            {
                minutos = 0;
                break;
            }
        } 

        sleep_ms(1000);

        }
        }
    }
}
