/*/
CRISTOVAO VINICIUS COSTA
Projeto proposto é um tipo de equipamento onde o som varia em 4 notas musicais
na variação do movimento do joystick nas entradas PWM, e perifericos comuns
um botao para ocorrer a troca de notas, um LED RBG  para diferenciar em qual nota esta 
no momento atual(pode ser mudado ao aperta a tecla um para uma rapida troca,) 


/*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// Definição dos pinos
#define LED_RED 13      // LED VERMELHO    "LÁ"
#define LED_GREEN 11    // LED VERDE       "DÓ"
#define LED_BLUE 12     //LED AZUL          "RÉ"
//LED NA COR BRANCA JUNÇÃO DOS 3            "MI"
#define BUZZER 10
#define JOYSTICK_X 27  // Pino ADC para eixo X
#define JOYSTICK_Y 26  // Pino ADC para eixo Y
#define BUTTON 5       // EQUIVALNETE BOTAO A BITDOGLAB,  DEFINIDO TAMBEM COMO TECLA 1 NO TECLADO DO COMPUTADOR

// Frequências para cada estado (4 notas diferentes)
const int frequencies[4] = {440, 523, 587, 659}; // Lá, Dó, Ré, Mi
const int ledPins[3] = {LED_RED, LED_GREEN, LED_BLUE}; // LEDs vermelho, verde e azul

// Variáveis globais
int currentState = 0; // Estado atual (0 a 3)
bool buttonPressed = false;

// Função para tocar o buzzer com uma frequência específica
void playBuzzer(int frequency) {
    if (frequency == 0) {
        gpio_put(BUZZER, 0); // Desliga o buzzer
        return;
    }
    int delayUs = 1000000 / (2 * frequency); // Calcula o delay para a frequência
    for (int i = 0; i < 100; i++) { // Toca por um curto período
        gpio_put(BUZZER, 1);
        sleep_us(delayUs);
        gpio_put(BUZZER, 0);
        sleep_us(delayUs);
    }
}

// Função para ligar/desligar LEDs conforme o estado atual
void updateLEDs() {
    if (currentState == 3) {
        // Liga todos os LEDs juntos no estado 3
        gpio_put(LED_RED, 1);
        gpio_put(LED_GREEN, 1);
        gpio_put(LED_BLUE, 1);
    } else {
        // Liga apenas o LED correspondente ao estado
        for (int i = 0; i < 3; i++) {
            gpio_put(ledPins[i], (i == currentState) ? 1 : 0);
        }
    }
}

// Função de interrupção do botão
void buttonISR(uint gpio, uint32_t events) {
    if (gpio == BUTTON) {
        buttonPressed = true;
    }
}

// Configuração inicial
void setup() {
    stdio_init_all();
    
    // Inicializa LEDs
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    // Inicializa o buzzer
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);

    // Inicializa joystick (X e Y)
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    // Inicializa botão
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &buttonISR);

    // Desliga LEDs e buzzer inicialmente
    gpio_put(LED_RED, 0);
    gpio_put(LED_GREEN, 0);
    gpio_put(LED_BLUE, 0);
    gpio_put(BUZZER, 0);
}

// Loop principal
void loop() {
    // Troca de estado quando o botão é pressionado
    if (buttonPressed) {
        currentState = (currentState + 1) % 4; // Alterna entre 0, 1, 2 e 3
        buttonPressed = false;
        updateLEDs(); // Atualiza os LEDs
    }

    // Lê os valores do joystick (X e Y)
    adc_select_input(0);
    uint16_t yValue = adc_read();
    adc_select_input(1);
    uint16_t xValue = adc_read();

    int buzzerFreq = 0;

    // Define o som baseado na posição do joystick
    if (yValue > 2500) {  // Joystick para cima
        buzzerFreq = frequencies[currentState]; // Nota do estado atual
    } 
    else if (yValue < 1500) {  // Joystick para baixo
        buzzerFreq = frequencies[currentState] / 2; // Metade da frequência
    } 
    else if (xValue > 2500) {  // Joystick para a direita
        buzzerFreq = frequencies[currentState] * 1.5; // Aumenta a frequência
    } 
    else if (xValue < 1500) {  // Joystick para a esquerda
        buzzerFreq = frequencies[currentState] / 1.5; // Reduz a frequência
    }

    // Toca o som correspondente
    playBuzzer(buzzerFreq);

    sleep_ms(100); // Pequeno delay para estabilidade
}

// Função principal
int main() {
    setup();
    while (true) {
        loop();
    }
}
