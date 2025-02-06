#include "includes.h"
#include "constantes.h"

struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};

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

void calc_render_area_buflen(struct render_area *area) {
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

#ifdef i2c_default

void SSD1306_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i=0;i<num;i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

void SSD1306_init() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t cmds[] = {
        SSD1306_SET_DISP,               // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                           // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,                           
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02,
#endif
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
        0x80,                           // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,          // set pre-charge period
        0xF1,                           // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
        0x30,                           // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST,           // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,        // set charge pump
        0x14,                           // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01, // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

void render(uint8_t *buf, struct render_area *area) {
    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}
#endif

void inicializar_pio(PIO pio, uint sm, uint offset){
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
}

void init_buzzer(int BUZZER) {
  gpio_set_function(BUZZER, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(BUZZER);

  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (FREQUENCIA_BUZZER * 4096));
  pwm_init(slice_num, &config, 1);

  pwm_set_gpio_level(BUZZER, 0);
}

void buzzer_alerta()
{
    pwm_set_gpio_level(BUZZER_A, FREQUENCIA_BUZZER);
    pwm_set_gpio_level(BUZZER_B, FREQUENCIA_BUZZER);
    sleep_ms(1000);
    pwm_set_gpio_level(BUZZER_A, 0);
    pwm_set_gpio_level(BUZZER_B, 0);
    sleep_ms(1000);
}

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

void inicializa_oled(){
    i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    SSD1306_init();

  struct render_area frame_area = {
      start_col: 0,
      end_col : SSD1306_WIDTH - 1,
      start_page : 0,
      end_page : SSD1306_NUM_PAGES - 1
      };

  calc_render_area_buflen(&frame_area);

  // zero the entire display
  static uint8_t buf[SSD1306_BUF_LEN];
  memset(buf, 0, SSD1306_BUF_LEN);
  render(buf, &frame_area);

  //render face
  for (int i=0; i<sizeof(face); i+=24) {
    memcpy(buf+i, &face[i], 24);
    render(buf, &frame_area);
    sleep_ms(1);
  }
}

void inicializa_bloco_expositor(PIO pio, uint sm)
{ 
    int cor_index = 0;
    uint32_t cores[4] = {
        urgb_u32(20, 0, 0),    // Vermelho (R, G, B)
        urgb_u32(0, 0, 20),    // Azul (R, G, B)
        urgb_u32(0, 20, 0),    // Verde (R, G, B)
        urgb_u32(20, 20, 20) // Branco (R, G, B)
    };
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
}
