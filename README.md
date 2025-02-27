Monitoramento Inteligente de Motores com Alertas em Tempo Real

Descrição:

Este projeto tem como objetivo monitorar o funcionamento de motores elétricos em tempo real, utilizando sensores para medir temperatura, rotação, vibração e corrente. O sistema é implementado com o microcontrolador RP2040 da Raspberry Pi, utilizando a placa de desenvolvimento BitDogLab. O projeto inclui a exibição dos estados dos sensores em um display OLED SSD1306, alertas visuais com LEDs RGB e alertas sonoros com um buzzer.

O sistema é capaz de detectar condições anormais, como sobrecarga, falhas mecânicas ou problemas de resfriamento, e emitir alertas em tempo real para evitar danos ao motor.

Objetivos:
Monitorar a temperatura, rotação, vibração e corrente de um motor elétrico.

Exibir os estados dos sensores em um display OLED SSD1306.

Utilizar LEDs RGB para indicar o status do motor (OK, moderado, crítico).

Emitir alertas sonoros com um buzzer em caso de condições críticas.

Implementar um sistema de interrupção para tratar eventos de botões.

Componentes Utilizados:
BitDogLab: Placa de desenvolvimento com RP2040.

LED RGB: Conectado às GPIOs 11 (Verde), 12 (Azul) e 13 (Vermelho).

Joystick: Conectado aos GPIOs 26 (eixo Y) e 27 (eixo X) para simular leituras de sensores.

Botão do Joystick: Conectado à GPIO 22.

Botão A: Conectado à GPIO 5.

Botão B: Conectado à GPIO 6.

Buzzer: Conectado à GPIO 21.

Display SSD1306: Conectado via I2C, utilizando os GPIOs 14 (SDA) e 15 (SCL).

Funcionamento:
1. Monitoramento dos Sensores
Temperatura:

Simulada pelo eixo Y do joystick (GPIO 27).

Valores:

0 a 1500: Estado moderado.

1501 a 3000: Estado OK.

Acima de 3000: Estado crítico.

Rotação:

Simulada pelo eixo X do joystick (GPIO 26).

Valores:

0 a 1500: Estado moderado.

1501 a 3000: Estado OK.

Acima de 3000: Estado crítico.

Vibração e Corrente:

Simuladas pelos botões A e B, que incrementam os valores de vibração e corrente, respectivamente.

2. Exibição no Display SSD1306
O display exibe o status dos sensores em tempo real:

Temperatura: OK, moderado ou crítico.

Rotação: OK, moderado ou crítico.

Vibração: OK, moderado ou crítico.

Corrente: OK, moderado ou crítico.

3. Alertas Visuais com LEDs RGB
LED Verde:

Acende quando todos os sensores estão no estado OK.

LED Azul:

Acende quando algum sensor está no estado moderado.

LED Vermelho:

Acende quando algum sensor está no estado crítico.

4. Alertas Sonoros com Buzzer
O buzzer emite um alerta sonoro quando o LED Vermelho está aceso (condição crítica).

5. Funcionalidades dos Botões
Botão do Joystick:

Reinicia os valores dos sensores para o estado OK.

Botão A:

Incrementa o valor da vibração.

Botão B:

Incrementa o valor da corrente.

Estrutura do Código
O código está organizado da seguinte forma:

Inicialização:

Configuração dos pinos GPIO, ADC e I2C.

Inicialização do display SSD1306.

Leitura dos Sensores:

Os valores dos eixos X e Y do joystick são lidos e convertidos em estados (OK, moderado, crítico).

Atualização do Display:

O display é atualizado com os estados dos sensores.

Controle dos LEDs:

Os LEDs são controlados com base nos estados dos sensores.

Alertas Sonoros:

O buzzer é ativado em caso de estado crítico.

Tratamento de Botões:

Os botões são tratados com debounce para evitar leituras falsas.

Como Executar o Projeto
Conecte os componentes conforme descrito na seção Componentes Utilizados.

Compile e carregue o código no microcontrolador RP2040.

Movimente o joystick para simular variações nos sensores.

Utilize os botões para incrementar os valores de vibração e corrente.

Observe os alertas no display, LEDs e buzzer.

Autor
Nome: Mateus Moreira da Silva

Repositório: Link para o repositório

Vídeo de Demonstração: Link para o vídeo
