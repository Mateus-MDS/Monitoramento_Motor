#include <stdio.h>             // Biblioteca padr�o para entrada e sa�da, utilizada para printf.
#include "pico/stdlib.h"       // Biblioteca padr�o para fun��es b�sicas do Pico, como GPIO e temporiza��o.
#include "hardware/adc.h"      // Biblioteca para controle do ADC (Conversor Anal�gico-Digital).
#include "hardware/i2c.h"      // Biblioteca para comunica��o I2C.
#include "inc/ssd1306.h"       // Biblioteca para controle do display OLED SSD1306.
#include "inc/font.h"          // Biblioteca para fontes usadas no display.
#include "hardware/clocks.h"   // Biblioteca para controle de clocks do Pico.
#include "pico/stdio_usb.h"    // Biblioteca para comunica��o USB.

#define I2C_PORT i2c1          // Define a porta I2C utilizada.
#define I2C_SDA 14             // Define o pino SDA para I2C.
#define I2C_SCL 15             // Define o pino SCL para I2C.
#define ENDERECO 0x3C          // Endere�o I2C do display OLED.

#define WIDTH 128              // Largura do display OLED.
#define HEIGHT 64              // Altura do display OLED.

// Defini��es dos pinos para o joystick, bot�es e LEDs
#define temperatura 27         // Pino GP27 para leitura da temperatura (ADC0).
#define rotacao 26             // Pino GP26 para leitura da rota��o (ADC1).
#define arrumado 22            // Pino GP22 para o bot�o do joystick (entrada digital).
#define LED_VERMELHO 13        // Pino GP13 para o LED vermelho.
#define LED_AZUL 12            // Pino GP12 para o LED azul.
#define LED_VERDE 11           // Pino GP11 para o LED verde.
#define BOTAO_A 5              // Pino GP5 para o bot�o A.
#define BOTAO_B 6              // Pino GP6 para o bot�o B.
#define BUZZER 21              // Pino GP21 para o buzzer.

// Vari�veis globais para armazenar valores dos sensores e estados
uint percent_rotacao;
uint percent_temperatura;
uint rotacao_value;
uint temperatura_value;
uint vibrando_value = 10;
uint corrente_value = 10;
uint valor_temperatura;
uint valor_vibracao;
uint valor_rotacao;
uint valor_corrente;
uint SISTEMA = 1;

// Strings para armazenar os estados dos sensores
const char* estado_temperatura;
const char* estado_rotacao;
const char* estado_vibracao;
const char* estado_corrente;

ssd1306_t ssd; // Vari�vel global para o display OLED.

// Par�metros para debounce
#define DEBOUNCE_DELAY_MS 50   // Tempo de debounce em milissegundos.

// Vari�veis para armazenar o estado dos bot�es
bool button_a_state = false;
bool button_b_state = false;

// Fun��o para verificar o debounce de um bot�o
bool debounce(uint pin, bool *last_state) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool current_state = gpio_get(pin) == 0; // 0 indica que o bot�o est� pressionado (ativo em n�vel baixo)

    if (current_state != *last_state) {
        if (current_time - last_time > DEBOUNCE_DELAY_MS) {
            last_time = current_time;
            *last_state = current_state;
            return current_state;
        }
    }
    return *last_state;
}

// Fun��o para atualizar o display com os estados dos sensores
void atualizar_display(const char* estado_temperatura, const char* estado_rotacao, const char* estado_vibracao, const char* estado_corrente) {
    ssd1306_fill(&ssd, false); // Limpa o display.
    ssd1306_draw_string(&ssd, "SITUACAO SENSOR", 5, 10); // Desenha o t�tulo.
    ssd1306_draw_string(&ssd, estado_temperatura, 10, 20); // Desenha o estado da temperatura.
    ssd1306_draw_string(&ssd, estado_rotacao, 10, 30); // Desenha o estado da rota��o.
    ssd1306_draw_string(&ssd, estado_vibracao, 10, 40); // Desenha o estado da vibra��o.
    ssd1306_draw_string(&ssd, estado_corrente, 10, 50); // Desenha o estado da corrente.
    ssd1306_send_data(&ssd); // Envia os dados para o display.
}

// Fun��o para atualizar os LEDs com base nos valores dos sensores
void atualizar_leds(uint valor_temperatura, uint valor_rotacao, uint valor_vibracao, uint valor_corrente) {
    gpio_put(LED_VERDE, (valor_temperatura == 0 && valor_rotacao == 0 && valor_vibracao == 0 && valor_corrente == 0)); // LED verde liga se tudo estiver OK.

    gpio_put(LED_VERMELHO, (valor_temperatura == 100 || valor_rotacao == 100 || valor_vibracao == 100 || valor_corrente == 100)); // LED vermelho liga se algum sensor estiver em estado cr�tico.
    gpio_put(LED_AZUL, false); // Desliga o LED azul.

    if((gpio_get(LED_VERMELHO)) == false){
        gpio_put(LED_AZUL, (valor_temperatura == 50 || valor_rotacao == 50 || valor_vibracao == 50 || valor_corrente == 50)); // LED azul liga se algum sensor estiver em estado moderado.
    }
}

// Fun��o para emitir um beep
void ALERTA_SONORO(){
    for (int i = 0; i < 100; i++) {
        gpio_put(BUZZER, 1); // Liga o buzzer.
        sleep_us(500);       // Espera 500 microssegundos.
        gpio_put(BUZZER, 0); // Desliga o buzzer.
        sleep_us(500);       // Espera 500 microssegundos.
    }
}

// Fun��o para controlar o estado da temperatura
void controle_temperatura(){
    if (temperatura_value <= 1500) {
        valor_temperatura = 50; // Estado moderado.
    } else {
        if (temperatura_value > 1500 && temperatura_value <= 3000) {
            valor_temperatura = 0; // Estado OK.
        } else {
            valor_temperatura = 100; // Estado cr�tico.
        }
    }
}

// Fun��o para controlar o estado da rota��o
void controle_rotacao(){
    if (rotacao_value <= 1500) {
        valor_rotacao = 50; // Estado moderado.
    } else {
        if (rotacao_value > 1500 && rotacao_value <= 3000) {
            valor_rotacao = 0; // Estado OK.
        } else {
            valor_rotacao = 100; // Estado cr�tico.
        }
    }
}

// Fun��o para desenhar bordas no display
void desenhar_bordas(ssd1306_t *ssd) {
    ssd1306_rect(ssd, 0, 0, WIDTH, HEIGHT, true, false); // Desenha uma borda ao redor do display.
    ssd1306_send_data(ssd); // Envia os dados para o display.
}

// Fun��o de interrup��o para os bot�es
void gpio_irq_handler(uint gpio, uint32_t events);

int main() {
    // Inicializa a comunica��o serial para permitir o uso de printf.
    stdio_init_all();
    stdio_usb_init();
    sleep_ms(2000); // Espera 2 segundos para estabiliza��o.

    // Inicializa o m�dulo ADC do Raspberry Pi Pico.
    adc_init();
    adc_gpio_init(rotacao); // Configura GP26 (ADC0) para o eixo X do joystick.
    adc_gpio_init(temperatura); // Configura GP27 (ADC1) para o eixo Y do joystick.

    // Configura o pino do bot�o do joystick como entrada digital com pull-up interno.
    gpio_init(arrumado);
    gpio_set_dir(arrumado, GPIO_IN);
    gpio_pull_up(arrumado);

    // Configura os bot�es A e B como entrada digital com pull-up interno.
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // Inicializa os pinos dos LEDs como sa�da e desliga-os inicialmente.
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, false);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, false);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, false);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);

    // Configura��o do I2C para o display SSD1306.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display SSD1306.
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configura interrup��es para os bot�es.
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(arrumado, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    uint32_t last_print_time = 0; // Vari�vel para controlar o tempo de impress�o na serial.

    bool cor = true;

    desenhar_bordas(&ssd); // Desenha a borda inicial no display.

    while (true) {
        // Leitura do valor do ADC para VRX (Eixo X do joystick).
        adc_select_input(0); // Seleciona canal 0 (GP26 - VRX).
        rotacao_value = adc_read(); // L� o valor do eixo X, de 0 a 4095.

        // Leitura do valor do ADC para VRY (Eixo Y do joystick).
        adc_select_input(1); // Seleciona canal 1 (GP27 - VRY).
        temperatura_value = adc_read(); // L� o valor do eixo Y, de 0 a 4095.

        // Leitura do estado do bot�o do joystick (SW).
        bool sw_value = gpio_get(arrumado) == 0; // 0 indica que o bot�o est� pressionado.

        // Verifica o estado dos bot�es com debounce.
        bool button_a_pressed = debounce(BOTAO_A, &button_a_state); // Verifica se o bot�o A foi pressionado com debounce.
        bool button_b_pressed = debounce(BOTAO_B, &button_a_state); // Verifica se o bot�o B foi pressionado com debounce.

        // Intera��o com os LEDs com base no movimento do joystick.
        controle_rotacao();
        controle_temperatura();
        
        if (SISTEMA == 1) {
            // Atualiza os estados dos sensores com base nos valores lidos.
            if (valor_temperatura == 0) {
                estado_temperatura = "TEMPERATURA OK";
            } else if (valor_temperatura == 50) {
                estado_temperatura = "TEMPERATURA MO";
            } else {
                estado_temperatura = "TEMPERATURA AL";
                printf("POSSIVEIS PROBLEMAS:\n");
                printf("SOBRECARGA NO MOTOR\n");
                printf("TENSAO INCONSISTENTE\n");
                printf("FALHA NO RESFRIAMENTO\n");
            }

            if (valor_rotacao == 0) {
                estado_rotacao = "ROTACAO OK";
            } else if (valor_rotacao == 50) {
                estado_rotacao = "ROTACAO MO";
            } else {
                estado_rotacao = "ROTACAO AL";
                printf("POSSIVEIS PROBLEMAS:\n");
                printf("FALHA NO INVERSOR DE FREQUENCIA\n");
                printf("FALHA MECANICA(EXCESSO OU FALTA DE CARGA)\n");
            }

            if (valor_vibracao == 0) {
                estado_vibracao = "VIBRACAO OK";
            } else if (valor_vibracao == 50) {
                estado_vibracao = "VIBRACAO MO";
            } else {
                estado_vibracao = "VIBRACAO AL";
                printf("POSSIVEIS PROBLEMAS:\n");
                printf("DESALINHAMENTO DOS ENROLAMENTOS\n");
                printf("DESGASTE DOS ENROLAMENTOS\n");
                printf("PROBLEMA DE ACLOPAMENTO\n");
            }

            if (valor_corrente == 0) {
                estado_corrente = "CORRENTE OK";
            } else if (valor_corrente == 50) {
                estado_corrente = "CORRENTE MO";
            } else {
                estado_corrente = "CORRENTE AL";
                printf("POSSIVEIS PROBLEMAS:\n");
                printf("SOBRECARGA NO MOTOR\n");
                printf("CURTO CIRCUITO NOS ENROLAMENTOS\n");
                printf("TENSAO ELETRICA INCONSISTENTE\n");
            }
        }

        // Atualiza o display e os LEDs com os estados dos sensores.
        atualizar_display(estado_temperatura, estado_rotacao, estado_vibracao, estado_corrente);
        atualizar_leds(valor_temperatura, valor_rotacao, valor_vibracao, valor_corrente);

        // Se o LED vermelho estiver ligado, emite um alerta sonoro.
        if(gpio_get(LED_VERMELHO)) {
            ALERTA_SONORO();
        }

        // Verifica se h� entrada do usu�rio via USB.
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            switch (c) {
                case 'D': // Desliga o sistema.
                    ssd1306_fill(&ssd, false); // Limpa o display.
                    ssd1306_draw_string(&ssd, "SISTEMA", 10, 30); // Desenha "SISTEMA".
                    ssd1306_draw_string(&ssd, "DESLIGADO", 10, 40); // Desenha "DESLIGADO".
                    ssd1306_send_data(&ssd); // Envia os dados para o display.
                    sleep_ms(5000); // Espera 5 segundos.
                    exit(0); // Encerra o programa.
                    break;

                case 'R': // Reinicia o sistema.
                    ssd1306_fill(&ssd, false); // Limpa o display.
                    ssd1306_draw_string(&ssd, "SISTEMA", 10, 30); // Desenha "SISTEMA".
                    ssd1306_draw_string(&ssd, "REINICIADO", 10, 40); // Desenha "REINICIADO".
                    ssd1306_send_data(&ssd); // Envia os dados para o display.
                    sleep_ms(5000); // Espera 5 segundos.
                    // Reinicia os valores dos sensores.
                    valor_temperatura = 0;
                    valor_rotacao = 0;
                    valor_corrente = 0;
                    valor_vibracao = 0;
                    vibrando_value = 10;
                    corrente_value = 10;
                    SISTEMA = 1;
                    break;
            }
        }

        // Imprime os valores lidos e o duty cycle proporcional na comunica��o serial a cada 1 segundo.
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000) {
            printf("VRX: %u, VRY: %u, SW: %d\n", rotacao_value, temperatura_value);
            printf("corrente_value: %d, vibra��o_value: %d\n", corrente_value, vibrando_value);
            last_print_time = current_time; // Atualiza o tempo da �ltima impress�o.
        }
    }
}

// Fun��o de interrup��o quando um dos bot�es forem pressionados.
void gpio_irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debouncing de 300ms.
    if (current_time - last_time > 300000) {
        last_time = current_time;

        // Verifica se o bot�o A foi pressionado.
        if (gpio == BOTAO_A && !gpio_get(BOTAO_A)) {
            // Aumenta o valor da vibra��o.
            vibrando_value += 10;
            if (vibrando_value == 0) {
                valor_vibracao = 0;
            } else if (vibrando_value == 30) {
                valor_vibracao = 50;
            } else if (vibrando_value == 50) {
                valor_vibracao = 100;
            }
        }

        // Verifica se o bot�o B foi pressionado.
        if (gpio == BOTAO_B && !gpio_get(BOTAO_B)) {
            // Aumenta o valor da corrente.
            corrente_value += 10;
            if (corrente_value == 0) {
                valor_corrente = 0;
            } else if (corrente_value == 30) {
                valor_corrente = 50;
            } else if (corrente_value == 50) {
                valor_corrente = 100;
            }
        }

        // Verifica se o bot�o do joystick foi pressionado.
        if (gpio == arrumado && !gpio_get(arrumado)) {
            // Reinicia os valores dos sensores.
            valor_temperatura = 0;
            valor_rotacao = 0;
            valor_corrente = 0;
            valor_vibracao = 0;
            vibrando_value = 10;
            corrente_value = 10;
            SISTEMA = 1;
        }
    }
}