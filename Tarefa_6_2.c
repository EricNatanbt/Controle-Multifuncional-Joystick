#include <stdio.h>
#include "ssd1306.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "funcoes.h"

// Função para ajustar o brilho dos LEDs com base nos valores do joystick
// Função principal
int main() {
    iniciarDisplay(); // Inicializa o display
    setup_joystick(); // Configura o joystick
    setup_leds();     // Configura os LEDs
    pwm_init_buzzer(BUZZER_PIN); // Inicializa o buzzer

    while (1) {
        int opcao = menu(); // Exibe o menu e obtém a opção selecionada

        switch (opcao) {
            case 1:
                joystick_led_control(); // Controle dos LEDs com o joystick
                break;
            case 2:
                 pwm_led_control(LED_GREEN, PERIOD, DIVIDER_PWM, LED_STEP); // Ajusta o brilho dos LEDs manualmente
                break;
            case 3:
                playing_buzzer_code(BUZZER_PIN); // Toca uma música no buzzer
                break;
            default:
                break;
        }
    }

    return 0;
}