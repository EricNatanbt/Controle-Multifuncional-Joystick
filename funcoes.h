#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "hardware/pwm.h" // Biblioteca para controle de PWM no RP2040
#include "hardware/clocks.h" // Biblioteca para controle dos clocks no RP2040
#include "ssd1306.h" // Biblioteca para o display SSD1306


//===========================Definições Globais==================================
// Definição dos pinos I2C
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Inicialização do display e área de renderização
struct render_area frame_area;
uint8_t ssd[ssd1306_buffer_length];

//===========================Definições BUZZER==================================

#define BUZZER_PIN 21 // Pino do buzzer

// Notas musicais para a melodia do Super Mario
const uint melody_notes[] = {
    659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0, 523, 0, 392, 0, 330, 0, 440, 0, 494, 0, 466, 440, 0, 392, 659, 784, 880, 698, 784, 0, 659, 0, 523, 587, 494, 0, 523, 0, 392, 0, 330, 0, 440, 0, 494, 0, 466, 440, 0, 392, 659, 784, 880, 698, 784, 0, 659, 0, 523, 587, 494
};

// Duração das notas (em milissegundos)
const uint melody_duration[] = {
    200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200
};

//===========================Definições JOYSTICK==================================

const int VRX = 26;          // Pino de leitura do eixo X do joystick (conectado ao ADC)
const int VRY = 27;          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
const int ADC_CHANNEL_0 = 1; // Canal ADC para o eixo X do joystick
const int ADC_CHANNEL_1 = 0; // Canal ADC para o eixo Y do joystick
const int JOYSTICK_BUTTON_PIN = 22; // Pino de leitura do botão do joystick

uint16_t vrx_value, vry_value; // Valores dos eixos do joystick


//===============================Definições LED=====================================

const int LED_BLUE = 12;    // Pino para controle do LED azul via PWM
const int LED_RED = 13;     // Pino para controle do LED vermelho via PWM
const int LED_GREEN = 11;   // Pino para controle do LED verde via PWM

const uint16_t LED_STEP = 160; 

const float DIVIDER_PWM = 16.0; // Divisor fracional do clock para o PWM
const uint16_t PERIOD = 4096;   // Período do PWM (valor máximo do contador)
uint16_t led_G_level, led_R_level; // Níveis de PWM para os LEDs

uint slice_led_G, slice_led_R; // Slices de PWM correspondentes aos LEDs

//===========================Funções do Display==================================
void LimpaDisplay(void) {
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void MensagemDisplay(const char *text[], int lines) {
    int y = 0;
    for (int i = 0; i < lines; i++) {
        ssd1306_draw_string(ssd, 5, y, (char *)text[i]);
        y += 8; // Avança para a próxima linha
    }
    render_on_display(ssd, &frame_area);
}

void iniciarDisplay(void) {
    stdio_init_all();

    // Inicialização do I2C e display
    i2c_init(i2c1, 400 * 1000); // 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    // Configuração da área de renderização
    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;

    calculate_render_area_buffer_length(&frame_area);
}


//=============================Função Menu principal=====================================

int menu(void) {
    LimpaDisplay();
    int menuP_index = 0; // Índice do menu
    const uint adc_max = (1 << 12) - 1; // Valor máximo do ADC (12 bits)
    const uint threshold = adc_max / 4; // Limite para considerar movimento

    while (1) {
        // Leitura do joystick
        adc_select_input(0); // Seleciona o canal 0 (eixo Y)
        uint adc_y_raw = adc_read(); // Lê o valor do eixo Y

        // Verifica se o joystick foi movido para cima ou para baixo
        if (adc_y_raw < threshold) {
            menuP_index += 1; // Move para a opção anterior
            sleep_ms(200); // Debounce
        } else if (adc_y_raw > adc_max - threshold) {
            menuP_index -= 1; // Move para a próxima opção
            sleep_ms(200); // Debounce
        }

        // Garante que o índice do menu permaneça dentro dos limites
        if (menuP_index < 0) menuP_index = 2;
        if (menuP_index > 2) menuP_index = 0;

        // Exibe o menu correspondente
        const char *text[6];
        switch (menuP_index) {
            case 0:
                text[0] = "Menu Principal"; text[1] = " "; text[2] = " "; text[3] = "i Joystick Led"; text[4] = "  PWM LED"; text[5] = "  Tocar Buzzer";
                break;
            case 1:
                text[0] = "Menu Principal"; text[1] = " "; text[2] = " "; text[3] = "  Joystick Led"; text[4] = "i PWM LED"; text[5] = "  Tocar Buzzer";
                break;
            case 2:
                text[0] = "Menu Principal"; text[1] = " "; text[2] = " "; text[3] = "  Joystick Led"; text[4] = "  PWM LED"; text[5] = "i Tocar Buzzer";
                break;
        }
        MensagemDisplay(text, 6); // Exibe o menu no display

        // Verifica se o botão do joystick foi pressionado
        if (!gpio_get(JOYSTICK_BUTTON_PIN)) {
            sleep_ms(200); // Debounce
            return menuP_index + 1; // Retorna a opção selecionada (1 a 3)
        }

        sleep_ms(100); // Aguarda um pouco antes de ler o joystick novamente
    }
}

//===========================Funções do BUZZER=====================================

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); // Ajusta divisor de clock
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Desliga o PWM inicialmente
}

void play_tone(uint pin, uint frequency, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2); // 50% de duty cycle

    sleep_ms(duration_ms);

    pwm_set_gpio_level(pin, 0); // Desliga o som após a duração
    sleep_ms(50); // Pausa entre notas
}

void playing_buzzer_code(uint pin) {
    LimpaDisplay();
    const char *text[6];
    text[0] = "TOCANDO BUZZER"; text[1] = " "; text[2] = " "; text[3] = "pressione o"; text[4] = "joystick para"; text[5] = "voltar ao menu";
    MensagemDisplay(text, 6);

    bool playing = true; // Variável para controlar se a música está tocando

    while (playing) {
        for (int i = 0; i < sizeof(melody_notes) / sizeof(melody_notes[0]); i++) {
            if (melody_notes[i] == 0) {
                pwm_set_gpio_level(pin, 0); // Desliga o buzzer para pausas na música
            } else {
                play_tone(pin, melody_notes[i], melody_duration[i]); // Toca a nota atual
            }

            // Verifica se o botão do joystick foi pressionado
            if (!gpio_get(JOYSTICK_BUTTON_PIN)) { // Se o botão for pressionado (LOW)
                playing = false; // Pausa a música
                break; // Sai do loop de reprodução
            }
        }
    }

    LimpaDisplay(); // Limpa o display ao sair
    pwm_set_gpio_level(pin, 0); // Desliga o buzzer
}

//===========================Funções do LED=======================================

void setup_pwm_led(uint led, uint *slice, uint16_t level) {
    gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino do LED como saída PWM
    *slice = pwm_gpio_to_slice_num(led);   // Obtém o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(*slice, DIVIDER_PWM);   // Define o divisor de clock do PWM
    pwm_set_wrap(*slice, PERIOD);          // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(led, level);        // Define o nível inicial do PWM para o LED
    pwm_set_enabled(*slice, true);         // Habilita o PWM no slice correspondente ao LED
}

void setup_leds() {
    setup_pwm_led(LED_BLUE, &slice_led_G, led_G_level); // Configura o PWM para o LED azul
    setup_pwm_led(LED_RED, &slice_led_R, led_R_level);  // Configura o PWM para o LED vermelho
    setup_pwm_led(LED_GREEN, &slice_led_G, led_G_level); // Configura o PWM para o LED verde
}

void clear_leds() {
    pwm_set_gpio_level(LED_GREEN, 0);
    pwm_set_gpio_level(LED_RED, 0);
    pwm_set_gpio_level(LED_BLUE, 0);
}

void adjust_led_brightness(uint16_t vrx_value, uint16_t vry_value) {
    pwm_set_gpio_level(LED_BLUE, vrx_value); // Ajusta o brilho do LED azul com o valor do eixo X
    pwm_set_gpio_level(LED_RED, vry_value);  // Ajusta o brilho do LED vermelho com o valor do eixo Y
}

// Função para controlar o brilho do LED via PWM
void pwm_led_control(uint led_pin, uint16_t period, float divider_pwm, uint16_t led_step) {
    uint16_t led_level = led_step; // Nível inicial do PWM (duty cycle)
    uint slice;
    uint up_down = 1; // Variável para controlar se o nível do LED aumenta ou diminui

    // Configuração do PWM
    gpio_set_function(led_pin, GPIO_FUNC_PWM); // Configura o pino do LED para função PWM
    slice = pwm_gpio_to_slice_num(led_pin);    // Obtém o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, divider_pwm);        // Define o divisor de clock do PWM
    pwm_set_wrap(slice, period);               // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(led_pin, led_level);    // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);              // Habilita o PWM no slice correspondente

    // Configuração do pino do botão do joystick
    const uint button_pin = 22;
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_pull_up(button_pin); // Habilita o resistor de pull-up interno (ajuste conforme necessário)

    // Exibe a mensagem inicial no display
    LimpaDisplay();
    const char *text[6];
    text[0] = "LED PWM"; text[1] = " "; text[2] = " "; text[3] = "pressione o"; text[4] = "joystick para"; text[5] = "voltar ao menu";
    MensagemDisplay(text, 6);

    // Loop para controlar o brilho do LED
    while (true) {
        pwm_set_gpio_level(led_pin, led_level); // Define o nível atual do PWM (duty cycle)
        sleep_ms(100); // Atraso de 100 ms (ajuste conforme necessário)

        // Verifica se o botão do joystick foi pressionado (com debounce simples)
        if (!gpio_get(button_pin)) { // Se o botão for pressionado (LOW)
            sleep_ms(50); // Debounce de 50 ms
            if (!gpio_get(button_pin)) { // Confirma se o botão ainda está pressionado
                LimpaDisplay(); // Limpa o display antes de sair
                break; // Sai do loop e, consequentemente, da função
            }
        }

        // Lógica para aumentar ou diminuir o brilho do LED
        if (up_down) {
            led_level += led_step; // Incrementa o nível do LED
            if (led_level >= period) {
                up_down = 0; // Muda direção para diminuir quando atingir o período máximo
            }
        } else {
            led_level -= led_step; // Decrementa o nível do LED
            if (led_level <= led_step) {
                up_down = 1; // Muda direção para aumentar quando atingir o mínimo
            }
        }
    }

    // Desliga o LED ao sair da função (opcional)
    pwm_set_gpio_level(led_pin, 0);
}

//===========================Funções do JOYSTICK===================================

void setup_joystick() {
    adc_init();         // Inicializa o módulo ADC
    adc_gpio_init(VRX); // Configura o pino VRX (eixo X) para entrada ADC
    adc_gpio_init(VRY); // Configura o pino VRY (eixo Y) para entrada ADC

    gpio_init(JOYSTICK_BUTTON_PIN);             // Inicializa o pino do botão
    gpio_set_dir(JOYSTICK_BUTTON_PIN, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(JOYSTICK_BUTTON_PIN);          // Ativa o pull-up no pino do botão
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value) {
    adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
    sleep_us(2);                     // Pequeno delay para estabilidade
    *vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)

    adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
    sleep_us(2);                     // Pequeno delay para estabilidade
    *vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)
}


void joystick_led_control() {
    uint16_t vrx_value, vry_value; // Variáveis para armazenar os valores do joystick (eixos X e Y)
LimpaDisplay();
  const char *text[6];
    text[0] = "Joystick led"; text[1] = " "; text[2] = " "; text[3] = "pressione o"; text[4] = "joystick para"; text[5] = "voltar ao menu";
    MensagemDisplay(text, 6);

    while (1) {
        joystick_read_axis(&vrx_value, &vry_value); // Lê os valores dos eixos do joystick
        adjust_led_brightness(vrx_value, vry_value); // Ajusta o brilho dos LEDs

        // Verifica se o botão do joystick foi pressionado para sair do modo de controle
        if (!gpio_get(JOYSTICK_BUTTON_PIN)) {
            sleep_ms(200); // Debounce
            clear_leds();
            break; // Sai do loop e retorna ao menu
        }

        sleep_ms(100); // Espera 100 ms antes de repetir o ciclo
    }
}
