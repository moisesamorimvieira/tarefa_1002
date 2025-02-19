
/*  
    Nome: Moises Amorim Vieira
    Matricula: tic370100277
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definindo constantes para pinos, endereço I2C e dimensões da tela OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_OLED 0x3C

#define WIDTH 128
#define HEIGHT 64

#define VRX_PIN 26
#define VRY_PIN 27
#define JOYSTICK_PB 22
#define BOTAO_A 5

#define LED_RED 11
#define LED_GREEN 12
#define LED_BLUE 13

// Função para inicializar o PWM em um GPIO
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

// Função para atualizar a tela OLED
void update_oled_display(ssd1306_t* ssd, int x, int y, bool thick_border) {
    ssd1306_fill(ssd, false); // Limpa a tela OLED
    ssd1306_rect(ssd, x, y, 8, 8, true, false); // Desenha um quadrado na posição x, y
    if (thick_border) {
        for (int i = 0; i < 3; i++) {
            ssd1306_rect(ssd, i, i, 127 - 2 * i, 63 - 2 * i, true, false); // Desenha uma borda grossa
        }
    } else {
        ssd1306_rect(ssd, 0, 0, 127, 63, true, false); // Desenha uma borda fina
    }
    ssd1306_send_data(ssd); // Envia os dados para a tela OLED
}

int main() {
    stdio_init_all(); // Inicializa o I/O padrão
    sleep_ms(2000); // Espera por 2 segundos

    // Inicializa e configura os pinos de entrada e saída
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, false);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, false);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, false);

    // Inicializa a comunicação I2C e configura o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO_OLED, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa os ADCs (conversores analógicos para digitais)
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    uint pwm_wrap = 4095;
    uint slice_red = pwm_init_gpio(LED_RED, pwm_wrap);
    uint slice_blue = pwm_init_gpio(LED_BLUE, pwm_wrap);

    uint16_t adc_value_x, adc_value_y;
    bool thick_border = false;
    bool leds_enabled = false;
    int square_x = WIDTH / 2 - 4;
    int square_y = HEIGHT / 2 - 4;

    // Loop principal
    while (true) {
        // Leitura dos valores do joystick
        adc_select_input(0);
        adc_value_x = adc_read();
        adc_select_input(1);
        adc_value_y = adc_read();

        // Mapeia os valores do joystick para a posição do quadrado na tela
        int new_x = adc_value_x * (WIDTH - 8) / 4095;
        int new_y = adc_value_y * (HEIGHT - 8) / 4095;

        // Verifica se a posição do quadrado mudou
        if (new_x != square_x || new_y != square_y) {
            leds_enabled = true;
            square_x = new_x;
            square_y = new_y;
        } else {
            leds_enabled = false;
        }

        // Atualiza os LEDs com base nos valores do joystick
        if (leds_enabled) {
            pwm_set_gpio_level(LED_RED, abs(adc_value_x - 2048) * 2);
            pwm_set_gpio_level(LED_BLUE, abs(adc_value_y - 2048) * 2);
        } else {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }

        // Verifica o estado do botão A
        bool a_state = !gpio_get(BOTAO_A);
        if (a_state) {
            leds_enabled = false;
            gpio_put(LED_RED, false);
            gpio_put(LED_BLUE, false);
            gpio_put(LED_GREEN, false);
            sleep_ms(300);
        }

        // Verifica o estado do botão do joystick
        bool pb_state = !gpio_get(JOYSTICK_PB);
        if (pb_state) {
            thick_border = !thick_border;
            gpio_put(LED_GREEN, thick_border);
            sleep_ms(300);
        }

        // Atualiza a tela OLED
        update_oled_display(&ssd, square_x, square_y, thick_border);

        printf("VRX: %u, VRY: %u, PB: %d, A: %d\n", adc_value_x, adc_value_y, pb_state, a_state);
        sleep_ms(100);
    }
}

