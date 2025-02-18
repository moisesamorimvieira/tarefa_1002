
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de pinos
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_OLED 0x3C

#define VRX_PIN 26  // ADC0
#define VRY_PIN 27  // ADC1
#define JOYSTICK_PB 22 
#define BOTAO_A 5  

#define LED1_PIN 11  
#define LED2_PIN 13  
#define LED3_PIN 12  

// Inicializa PWM no pino do LED
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Aguarda inicialização da USB

    // Configuração dos botões
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // Configuração dos LEDs
    gpio_init(LED1_PIN);
    gpio_set_dir(LED1_PIN, GPIO_OUT);
    gpio_put(LED1_PIN, false);

    gpio_init(LED2_PIN);
    gpio_set_dir(LED2_PIN, GPIO_OUT);
    gpio_put(LED2_PIN, false);

    gpio_init(LED3_PIN);
    gpio_set_dir(LED3_PIN, GPIO_OUT);
    gpio_put(LED3_PIN, false);

    // Inicialização do I2C e display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO_OLED, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd);

    // Inicialização do ADC
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    // Configuração do PWM para controle do LED3
    uint pwm_wrap = 4096;
    uint pwm_slice = pwm_init_gpio(LED3_PIN, pwm_wrap);

    uint16_t adc_value_x, adc_value_y;
    char str_x[6], str_y[6], str_pb[6], str_a[6];

    bool cor = true;
    while (true) {
        // Leitura do joystick
        adc_select_input(0); // ADC0 = VRX_PIN
        adc_value_x = adc_read();
        adc_select_input(1); // ADC1 = VRY_PIN
        adc_value_y = adc_read();

        // Leitura dos botões
        bool pb_state = !gpio_get(JOYSTICK_PB);
        bool a_state = !gpio_get(BOTAO_A);

        // Controle de LEDs com base nos valores do joystick
        gpio_put(LED1_PIN, adc_value_x > 2100);
        gpio_put(LED2_PIN, adc_value_y > 2100);
        gpio_put(LED3_PIN, pb_state);

        // Controle do PWM no LED3
        pwm_set_gpio_level(LED3_PIN, adc_value_x);

        // Conversão dos valores para string
        sprintf(str_x, "%d", adc_value_x);
        sprintf(str_y, "%d", adc_value_y);
        sprintf(str_pb, "%d", pb_state);
        sprintf(str_a, "%d", a_state);

        // Atualização do display OLED
        ssd1306_fill(&ssd, false); // Limpa o display

        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor); // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor); // Desenha uma linha  
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);
        ssd1306_draw_string(&ssd, "ADC   JOYSTICK", 10, 28);
        ssd1306_draw_string(&ssd, "X     Y     PB  A", 10, 41);
        ssd1306_line(&ssd, 44, 37, 44, 60, cor); // Desenha uma linha vertical         
        ssd1306_draw_string(&ssd, str_x, 8, 52); // Desenha uma string     
        ssd1306_line(&ssd, 84, 37, 84, 60, cor); // Desenha uma linha vertical      
        ssd1306_draw_string(&ssd, str_y, 49, 52); // Desenha uma string   
        ssd1306_rect(&ssd, 52, 90, 8, 8, cor, !gpio_get(JOYSTICK_PB)); // Desenha um retângulo  
        ssd1306_rect(&ssd, 52, 102, 8, 8, cor, !gpio_get(BOTAO_A)); // Desenha um retângulo    
        ssd1306_rect(&ssd, 52, 114, 8, 8, cor, !cor); // Desenha um retângulo       
        ssd1306_send_data(&ssd); // Atualiza o display

        /*ssd1306_draw_string(&ssd, str_x, 8, 52);
        ssd1306_draw_string(&ssd, str_y, 49, 52);
        ssd1306_draw_string(&ssd, str_pb, 80, 52);
        ssd1306_draw_string(&ssd, str_a, 100, 52);
        ssd1306_send_data(&ssd);*/

        
        printf("VRX: %u, VRY: %u, PB: %d, A: %d\n", adc_value_x, adc_value_y, pb_state, a_state);
        
        sleep_ms(100);
    }
}


