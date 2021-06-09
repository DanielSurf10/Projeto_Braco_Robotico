#include <Arduino.h>
#include "Servo.h"
#include "lib.h"
#include "PS2X_lib.h"

PS2X ps2x; // create PS2 Controller Class

// Defines
// Menus
#define PRIMEIRO     0
#define MUDANDO      1
#define EXECULTAR    2
#define EXECULTANDO  3
#define ADICIONAR    4
#define ADICIONANDO  5
#define PERGUNTA_ADC 6
#define APAGAR       7

// Pinos
// Analógicos - Potênciometros
#define SERVO_1 map(analogRead(A0), 0, 1023, 0, 180)
#define SERVO_2 map(analogRead(A1), 0, 1023, 0, 90)
#define SERVO_3 map(analogRead(A2), 0, 1023, 0, 90)
#define SERVO_4 map(analogRead(A3), 0, 1023, 50, 0)

// Servos
#define PINO_SERVO_1 4
#define PINO_SERVO_2 5
#define PINO_SERVO_3 6
#define PINO_SERVO_4 7

// Controle
#define BOTAO_X  ps2x.ButtonPressed(PSB_BLUE)
#define BOTAO_O  ps2x.ButtonPressed(PSB_RED)
#define BOTAO_A  ps2x.ButtonPressed(PSB_GREEN)

// Variáveis
// Sequencia
estado sequencia[100];        // Vetor da sequência
unsigned char quantidade = 0; // Quantidade de estados na sequência
bool vai_volta = false;       // Configuração se a sequência vai e volta ou só vai e reinicia
bool continua = false;        // para ver se continua ou não

// Botões
// Ultimo estado
bool BX_last = false,    // Botão X
     BO_last = false;    // Botão O
// Borda de subida
bool BX_APERTADO = false,    // Botão X apertado
     BO_APERTADO = false;    // Botão O apertado

// Contador
unsigned char executador = 0;     // Contador os estados da sequência
unsigned long contador_milis = 0; // contador do milis
unsigned int diferenca_milis = 0; // diferenca do contador
bool comecar_tempo = false;       // Começar a contar tempo
bool frente_tras = true;          // Contagem crescente ou decrescente - false desce - true sobe

// Menu
char menu = 0; // Numero do menu
bool menu_foi = false; // Menu já apareceu - lógica invertida

// Declaração dos Servos
Servo servo_1,
      servo_2,
      servo_3,
      servo_4;


// Protótipo de função
// Funções para Menu
void MenuInicial();     // Escrever no Serial o menu inicial
void MenuAtual();       // Atualiza o menu atual

// Funções para botões
void AtulizaBotoes();


void setup() {
  // Inicia o Serial
  Serial.begin(9600);

  // Escreve Inicialização Serial
  MenuInicial();

  // Define o valor do menu com zero
  menu = PRIMEIRO;

  // Verifica se o controle está acessível se não erro = 1
  if(ps2x.config_gamepad(13, 11, 10, 12, true, true) != 0) {
    Serial.println("Controle não encontrado");
    // Se o controle não foi encontrado não execulta mais nada
    while (1);
  }

  // Bagulho do Servo
  servo_1.attach(PINO_SERVO_1, 0, 180);
  servo_2.attach(PINO_SERVO_2, 0, 90);
  servo_3.attach(PINO_SERVO_3);
  servo_4.attach(PINO_SERVO_4, 0, 50);

  servo_1.write(100);
  servo_2.write(0);
  servo_3.write(0);
  servo_4.write(0);

}

void loop() {
  // Atualiza o estado do controle
  ps2x.read_gamepad(false, false);

  // Função para atualizar o Menu
  MenuAtual();

  // Verficação
  // Verfica se o botão X foi apertado 
  if (BOTAO_X) {

    // Muda para adicionar ou mudar se estiver no menu inicial
    if (menu == PRIMEIRO) {
      if (quantidade > 0) {
        Serial.println("Já configurado");
      } 

      else {
        menu = ADICIONAR;
        menu_foi = true;
      }
      
    }

    // Se estiver no Menu Apagar
    if (menu == APAGAR) {
      quantidade = 0;                         // Apagar a sequência
      menu = PRIMEIRO;                        // Voltar ao menu inicial
      menu_foi = true;                        // Liberar para mostrar o menu
      Serial.println("Sequencia apagada\n");  // Fala que a sequencia foi apagada
    }

    // Se tiver adicionando a sequência, adiciona o estado na sequência
    if (menu == ADICIONANDO) {
      sequencia[quantidade].ang_1 = SERVO_1;
      sequencia[quantidade].ang_2 = SERVO_2;
      sequencia[quantidade].ang_3 = SERVO_3;
      sequencia[quantidade].ang_4 = SERVO_4;

      quantidade++;

      menu_foi = true;                      // Liberar para mostrar o menu
    }

    // Se tiver execultando a sequência, incrementa o contador para o próximo estado
    if (menu == EXECULTANDO) {

      // Contador crescente ou decrescente
      if (vai_volta) {
        if (frente_tras)
          executador++;
        else
          executador--;
        
        if (((executador + 1) >= quantidade) && continua) {
          frente_tras = false;
        }

        if (executador == 0) {
            frente_tras = true;
        }
      }

      else {
        if (((executador + 1) >= quantidade) && continua)
          executador = 0;
        else
          executador++;
      }
      
      if ((!continua) && (((executador) >= quantidade) || (executador == 1 && frente_tras))) {
        menu = PRIMEIRO;
        executador = 0;
      }

      menu_foi = true;
    }

    // Se tiver na pergunta do adiciona, vai e volta recebe true
    if (menu == PERGUNTA_ADC) {
      vai_volta = true;
      menu = PRIMEIRO;
      
      menu_foi = true;
      Serial.println("Vai e volta selecionado\n");
    }

  }

  // Verfica se o botão O foi apertado
  if (BOTAO_O) {
    
    // Se a quantidade for zero então não a sequencia não foi configurada
    if (quantidade == 0) {
      Serial.println("Sequencia nao configurada");
    }

    // Se a sequencia foi configurada e tiver no 1º menu, vai pro menu execultar
    else if (menu == PRIMEIRO) {
      executador = 0;
      menu = EXECULTAR;
      menu_foi = true;    // Liberar para mostrar o menu
    }

    // Se estiver no Menu apagar
    if (menu == APAGAR) {
      menu = PRIMEIRO;                            // Voltar ao menu Inicial
      menu_foi = true;                            // Liberar para mostrar o menu
      Serial.println("Sequencia nao apagada\n");  // Fala que a sequencia não foi apagada
    }

    // Se tiver na pergunta do adiciona, vai e volta recebe false
    if (menu == PERGUNTA_ADC) {
      vai_volta = false;
      menu = PRIMEIRO;
      
      menu_foi = true;
      Serial.println("Vai e comeca de novo selecionado\n");
    }
  }

  // Verfica se o botão A (Triângulo) foi apertado
  if (BOTAO_A) {
    // Verifica se está no primeiro menu
    if (menu == PRIMEIRO) {
      // Quantidade é 0
      if (quantidade == 0) {
        Serial.println("Nao ha sequencia");
      } else {
        menu = APAGAR;
        menu_foi = true;
      } 
    }

    // Verifica se está adicionando a sequência se sim, para de adicionar
    if (menu == ADICIONANDO) {
      Serial.println("Nao tem");
      menu = PERGUNTA_ADC;
      menu_foi = true;
    }

    // Verifica se está execultano se sim para de execultar
    if (menu == EXECULTANDO) {
      continua = false;
    }
  }

  // Vai colocando o angulo
  if (menu == ADICIONANDO) {
    servo_1.write(SERVO_1);
    servo_2.write(SERVO_2);
    servo_3.write(SERVO_3);
    servo_4.write(SERVO_4);
  }
}


// Funções
// Menu Inicial
void MenuInicial() {
  Serial.println("--------------------");
  Serial.println("------Programa------");
  Serial.println("------do Braco------");
  Serial.println("------Robotico------");
  Serial.println("--------------------");
  Serial.println("Selecione:");
  Serial.println("  |-> [ X ] Adionar Sequencia");
  Serial.println("  |-> [ O ] Executar Sequencia");
  Serial.println("  |-> [ A ] Apagar Sequencia");
}

// Atualiza o menu atual
void MenuAtual() {

  // Se o menu ainda não apareceu
  if (menu_foi) {

    // switch para cada menu
    switch (menu) {

      // Menu Inicial
      case PRIMEIRO:
        MenuInicial();      // Mostar o menu inicial
        
        menu_foi = false;   // tornar falso o menu_foi
        break;

      // Execultar a sequência
      case EXECULTAR:
        // Começar a execultar 
        Serial.println("--------------------");
        Serial.println("Executando:");
        continua = true;
        menu = EXECULTANDO;

        break;
      
      // Executando a sequência
      case EXECULTANDO:
        Serial.print("Posicao ");
        Serial.print(executador + 1);
        Serial.print(": ");
        Serial.print(sequencia[executador].ang_1);
        Serial.print(" ");
        Serial.print(sequencia[executador].ang_2);
        Serial.print(" ");
        Serial.print(sequencia[executador].ang_3);
        Serial.print(" ");
        Serial.println(sequencia[executador].ang_4);

        servo_1.write(sequencia[executador].ang_1);
        servo_2.write(sequencia[executador].ang_2);
        servo_3.write(sequencia[executador].ang_3);
        servo_4.write(sequencia[executador].ang_4);

        menu_foi = false;   // tornar falso o menu_foi
        break;
      
      // Adicionar sequência
      case ADICIONAR:
        Serial.println("Criar sequencia:");
        Serial.println("Coloque na posicao e aperte [ X ]");
        Serial.println("Para parar precione [ A ]");
        Serial.print("\nPosicao 1: ");
        menu = ADICIONANDO;

        menu_foi = false;   // tornar falso o menu_foi
        break;
      
      // Adicionando Sequência
      case ADICIONANDO:
        Serial.print(sequencia[quantidade - 1].ang_1);
        Serial.print(" ");
        Serial.print(sequencia[quantidade - 1].ang_2);
        Serial.print(" ");
        Serial.print(sequencia[quantidade - 1].ang_3);
        Serial.print(" ");
        Serial.println(sequencia[quantidade - 1].ang_4);
        Serial.print("Posicao ");
        Serial.print(quantidade + 1);
        Serial.print(": ");

        menu_foi = false;   // tornar falso o menu_foi
        break;
      
      // Pergunta se vai e volta ou se vai e reinicia
      case PERGUNTA_ADC:
        Serial.println("Sequência vai e volta          [ X ]");
        Serial.println("Sequência vai e comeca de novo [ O ]");

        menu_foi = false;   // tornar falso o menu_foi
        break;
      
      // Pergunta se tem certeza, se sim apaga a sequencia
      case APAGAR:
        Serial.print("Tem certeza que quer apagar a sequencia? [ SIM -> X ] [ NAO -> O ] -> ");
        
        menu_foi = false;   // tornar falso o menu_foi
        break;

      default:
        break;
    }
  }

}
