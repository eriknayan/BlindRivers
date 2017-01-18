/* BLIND RIVER'S ELECTRONIC POKER GAME ©

Programa que controla e mostra os blinds do poker automaticamente,
através da configuração prévia do tempo de mudança.
Controla quem está no jogo, de quem é a vez e qual a rodada atual.
Tudo através de leds, buzzers, teclado e um display LCD.


Versão beta 1 - 26/02/2014  - Início da programação 
                            - Configuração da sequência de blinds
                            - Configuração do display LCD
                            - Contador de tempo restante para o próximo blind
                        
Versão beta 2 - 11/03/2014  - Implementação do botão de pause
                            - Sistema de jogador ativo, fold, dealer e vez
                            - Mudança no sistema de contagem de tempo
                            - Sistema de rodadas
                        
Versão beta 3 - 13/03/2014  - Finalizado o contador de tempo restante com pause e reinicio
                            - Desenvolvido hardware adicional do botão de fold
                            
Versão beta 4 - 14/03/2014  - Loop principal
                            - Agrupamento por funções
                            - Interrupção da vez utilizando MUX
                            - Interrupção de nova rodada e de flop/turn/river
                            
Versão beta 5 - 19/03/2014  - Organização da estrutura do programa
                            - Correções do loop principal baseados no teste com MSP
                            - Modificações em algumas variáveis
                            - Start implementado ao se pressionar o "*"
                            - Interrupção de fim de jogo e tempo total aproximado
                          
Versão beta 6 - 19/03/2014  - Configuração do buzzer de fold utilizando DEMUX
                            - Implementação das funções de boas vindas e start
                            
Versão beta 7 - 23/07/2014  - Primeiros testes com display, buzzer e teclado
                            - Corrigidos alguns bugs e aprimorada a tela de inicialização
                            - Modificação e correção da função start 
                            - Funções testadas marcadas

Versão beta 8 - 31/08/2014  - Organização dos pinos para o hardware do projeto
                            - Primeiros testes com os módulos dos jogadores
                            - Corrigidos bugs do sistema de vez do jogo
                            - Implementado novo sistema para reset (transistor de pull-up no SCR)
                            
Versão beta 9 - 02/09/2014  - Teste do sistema completo com as diversas possibilidades de ações
                            - Melhoria das funções de controle da vez (small blind sempre primeiro)
                            - Correção dos últimos bugs do sistema base
                            - Fase de testes final
 
Versão 1.0 - 06/09/2014     - Protótipo finalizado, funcionando 100%

Versão 1.1 - 04/10/2014     - Comunicação com Display LCD apenas com 3 fios, utilizando novas bibliotecas e um registrador 4094
                            
Desenvolvido por Erik Nayan.
All Rights Reserved.
*/

#include <LiquidCrystal_SR.h>
#include <Notes.h>
#include <Keypad.h>
#include <Timer.h>
#include <LCD.h>

// inicialização do display e seus pinos de comunicação
LiquidCrystal_SR lcd(12, 11, 13);

const byte Coluna = 3; //definição do número de colunas do teclado
const byte Linha = 4; //definição do número de linhas do teclado

int buzzer_blind = 4; // pino do buzzer
int pause = 2; //interrupção
int i; // variável auxiliar de for padrão

Timer t;

// definição dos pinos que farão a leitura se o jogador deu FOLD na rodada;
int FOLD[8] = {46,47,48,49,50,51,52,53};
// pino que passará a vez;  
int CHECK_BET_RAISE = 19;//interrupção

int LED_VEZ[8] = {30,31,32,33,34,35,36,37};

int LED_ON[8] = {38,39,40,41,42,43,44,45}; 

// pino indicador da mudança de rodada;
int FLOP_TURN_RIVER = 21;//interrupção

int VEZ; // Define quem é o small blind, exceto se estiver na primeira rodada
int VEZAT; // Jogador na vez atualmente
char DEALER; // Dealer da rodada
int RESET_RODADA = 3; //interrupção 

// Pinos de controle do enable do mux
int E2 = 7;
int E1 = 6;
int E0 = 5;

// variáveis usadas nas interrupções
volatile int state = LOW;
volatile long int deb = 0;
volatile int sairint = 0;
volatile int road_dealer; //dealer em int
volatile boolean CHECK;

int tempoblind;
int minutos;
int segundos;

int pause_on = 22;

int mudblind = 1; // contador de mudança do blind

char Teclas[Linha][Coluna] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}}; // atribuição das teclas 

// pinos do teclado - alterar
byte Pino_linha[Linha] = {15, 16, 17, 18};
byte Pino_coluna[Coluna] = {29, 28, 14}; 

Keypad teclado = Keypad(makeKeymap(Teclas), Pino_linha, Pino_coluna, Linha, Coluna ); //configurando o teclado

int bn = 0; // variável que conta se os dois digitos do tempo do blind foram adicionados
char bMS; // digito mais importante
char bLS; // digito menos importante

boolean d; // variável que verifica se o dealer inicial foi adicionado
boolean com; // variável que verifica se o jogo pode começar

void setup(){ 
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);  
  Serial.begin(9600);
  teclado.setDebounceTime(50); // tempo de debounce do teclado
  
  // definição do modo de todas as I/O
  
  // pinos de interrupções
  pinMode(pause, INPUT_PULLUP);
  pinMode(RESET_RODADA, INPUT_PULLUP); 
  pinMode(FLOP_TURN_RIVER, INPUT_PULLUP);
  pinMode(CHECK_BET_RAISE, INPUT_PULLUP);
  
  pinMode(E2, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(E0, OUTPUT);
  
  pinMode(FOLD[0], INPUT);
  pinMode(FOLD[1], INPUT);
  pinMode(FOLD[2], INPUT);
  pinMode(FOLD[3], INPUT);
  pinMode(FOLD[4], INPUT);
  pinMode(FOLD[5], INPUT);
  pinMode(FOLD[6], INPUT);
  pinMode(FOLD[7], INPUT);
  
  pinMode(LED_ON[0], OUTPUT);
  pinMode(LED_ON[1], OUTPUT);
  pinMode(LED_ON[2], OUTPUT);
  pinMode(LED_ON[3], OUTPUT);
  pinMode(LED_ON[4], OUTPUT);
  pinMode(LED_ON[5], OUTPUT);
  pinMode(LED_ON[6], OUTPUT);
  pinMode(LED_ON[7], OUTPUT); 
  
  pinMode(LED_VEZ[0], OUTPUT);
  pinMode(LED_VEZ[1], OUTPUT);
  pinMode(LED_VEZ[2], OUTPUT);
  pinMode(LED_VEZ[3], OUTPUT);
  pinMode(LED_VEZ[4], OUTPUT);
  pinMode(LED_VEZ[5], OUTPUT);
  pinMode(LED_VEZ[6], OUTPUT);
  pinMode(LED_VEZ[7], OUTPUT);
  
  pinMode(buzzer_blind, OUTPUT);
  pinMode(pause_on, OUTPUT);
    
  attachInterrupt(0, pausefunc , FALLING); 
  attachInterrupt(1, nova_rodada, FALLING);
  attachInterrupt(2, ftr, FALLING);
  attachInterrupt(4, passa_vez, FALLING);
  
  t.every(1000, tempcont);
 
  boas_vindas();

  configuration();

}
 
void loop(){
 
  t.update();
  CHECK=false;
  mux_enable();
  
  if(sairint!=1){    
    while(digitalRead(FOLD[VEZAT])!=LOW&&CHECK==false){
      t.update();
      verifica_tempo();
      digitalWrite(LED_ON[VEZAT],HIGH);
      digitalWrite(LED_VEZ[VEZAT],LOW); 
      imprimeblinds();
    
    }
  
    digitalWrite(LED_VEZ[VEZAT],HIGH);
  
    if(CHECK==true&&FOLD[VEZAT]!=LOW)
       digitalWrite(LED_ON[VEZAT],LOW);
    else
      digitalWrite(LED_ON[VEZAT],HIGH);

    imprimeblinds();
    verifica_tempo();
    
    VEZAT++;
    if(VEZAT>7)
      VEZAT-=8;
    
    mux_enable();
    
  }
  
}

void boas_vindas(){ 

  // deixa todos os led's no modo on (verde)
  for(i=0;i<8;i++){
        digitalWrite(LED_VEZ[i],HIGH); 
        digitalWrite(LED_ON[i],LOW);
  }
  
  // tela de boas vindas que imprime blind rivers "aleatoriamente";
  delay(1000);
  lcd.setCursor(2,0);
  lcd.print("B");
  delay(200);
  lcd.setCursor(8,0);
  lcd.print("R");
  delay(200);
  lcd.setCursor(4,0);
  lcd.print("i");
  delay(200);
  lcd.setCursor(10,0);
  lcd.print("v");
  delay(200);
  lcd.setCursor(6,0);
  lcd.print("d");
  delay(200);
  lcd.setCursor(12,0);
  lcd.print("r");
  delay(200);
  lcd.setCursor(5,0);
  lcd.print("n");
  delay(200);
  lcd.setCursor(14,0);
  lcd.print("s");
  delay(200);
  lcd.setCursor(9,0);
  lcd.print("i");
  delay(200);
  lcd.setCursor(3,0);
  lcd.print("l");
  delay(200);
  lcd.setCursor(11,0);
  lcd.print("e");
  delay(200);
  lcd.setCursor(13,0);
  lcd.print("'");
  delay(200);
  
  //Riff - Smoke On The Water - Deep Purple
  tone(buzzer_blind, NOTE_D4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_F4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_G4);
  delay(800);
  noTone(buzzer_blind);
  delay(300);
  tone(buzzer_blind, NOTE_D4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_F4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_GS4);
  delay(300);
  noTone(buzzer_blind);
  delay(10);
  tone(buzzer_blind, NOTE_G4);
  delay(800);
  noTone(buzzer_blind);
  delay(300);
  tone(buzzer_blind, NOTE_D4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_F4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_G4);
  delay(800);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_F4);
  delay(500);
  noTone(buzzer_blind);
  delay(150);
  tone(buzzer_blind, NOTE_D4);
  delay(1200);
  noTone(buzzer_blind);
  
  lcd.setCursor(0,1);
  lcd.print("   Poker Game");
  delay(1000);
  lcd.clear();
 
}

void configuration(){ 

  bn=0;
  d=false;
  lcd.setCursor(0,0);
  lcd.print("Tempo do blind:");
  lcd.setCursor(0,1);
  
  char teclaPressionada = teclado.getKey(); 

   while(bn!=2){   
     char teclaPressionada = teclado.getKey(); 
      if(teclaPressionada != NO_KEY && teclaPressionada != '$' && teclaPressionada != '*' && teclaPressionada != '#' && bn==0){
        bMS=teclaPressionada;
        teclaPressionada='$';
        lcd.print(bMS);
        bn++;
      }
  
      if(teclaPressionada != NO_KEY && teclaPressionada != '$' && teclaPressionada != '*' && teclaPressionada != '#' && bn==1){
        bLS=teclaPressionada;
        teclaPressionada='$';
        lcd.setCursor(1,1);
        lcd.print(bLS);
        bn++;
      } 
   }
   
  delay(700);
  lcd.clear();  
  
  // tempo do blind convertido de char para int manualmente
  tempoblind=(bMS-48)*10+(bLS-48);
  
  minutos=tempoblind;
  segundos=0;
  
  lcd.setCursor(0,0);
  lcd.print("Primeiro Dealer?");
  lcd.setCursor(0,1);
  lcd.print("Jogador: ");
  
  while(d==false){
    char teclaPressionada = teclado.getKey();
    if(teclaPressionada != NO_KEY && teclaPressionada != '*' && teclaPressionada != '#' && teclaPressionada != '0' && teclaPressionada != '9'){
      DEALER=teclaPressionada;
      lcd.print(DEALER);
      d=true;
    }
  }
  
  // conversão manual de um char para um int
  road_dealer=DEALER-48;
  
  delay(700);
  
  lcd.clear();
  start();
 
}

void start(){ 
  
  com=false;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("    MONEY ON");
  lcd.setCursor(0,1);
  lcd.print("   THE TABLE !!");
  delay(1000);
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("* = Iniciar");
  lcd.setCursor(0,1);
  lcd.print("# = Reconfigurar");

 
  char teclaPressionada = '$'; 
  if(teclaPressionada != NO_KEY){
    char teclaPressionada = teclado.getKey(); 
    while(com==false){
      char teclaPressionada = teclado.getKey(); 
      if(teclaPressionada=='*')
        com=true;   
      else if(teclaPressionada=='#'){
        lcd.clear();
        configuration();  
      }
    }
  } 
  
  lcd.clear();

  verifica_vez();
  
}

void verifica_vez(){
  
  VEZ=road_dealer+1;
  if(VEZ>8)
     VEZ-=8;

  VEZAT=VEZ-1;
  mux_enable();
  
}

void nova_rodada(){
  
  if(sairint!=1){
    digitalWrite(LED_VEZ[VEZAT],HIGH);
    
    for(i=0;i<8;i++)
      digitalWrite(LED_ON[i],LOW); 
      
    road_dealer++;
    if(road_dealer>8)
      road_dealer-=8;
      
    while(digitalRead(FOLD[road_dealer-1])==LOW){
      road_dealer++;
      if(road_dealer>8)
        road_dealer-=8;
    }
    
    verifica_vez();
    
  }
  
}

void ftr(){
   
    if(sairint!=1){
      digitalWrite(LED_VEZ[VEZAT],HIGH);
      digitalWrite(LED_ON[VEZAT],LOW); 
      VEZAT=VEZ-1;
      mux_enable();
    }
    
}

void passa_vez(){
    
  if(sairint!=1)
    CHECK=true;

}

void pausefunc() {
  
  state=!state;

  while(deb<100000){
    deb++;
  }

  deb=0;  
  
  sairint++;
  if(sairint==1)
    digitalWrite(pause_on, HIGH);
  else
    digitalWrite(pause_on, LOW);
    
  if(sairint==2)
    sairint=0;
      
}
  
void verifica_tempo(){
  
   if(minutos==0&&segundos==0){
      minutos=tempoblind;
      segundos=0;
      mudblind++;
      tone(buzzer_blind, NOTE_A4);
      delay(500);
      tone(buzzer_blind, NOTE_D4);
      delay(400);
      noTone(buzzer_blind);
      tone(buzzer_blind, NOTE_A4);
      delay(500);
      tone(buzzer_blind, NOTE_D4);
      delay(400);
      noTone(buzzer_blind);
   }
   
}

void imprimeblinds(){ 
  
    if(mudblind==1){ 
      lcd.setCursor(0,0);
      lcd.print("Blinds: 5/10");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }

    else if(mudblind==2){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 10/20");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
    
    else if(mudblind==3){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 15/30");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
  
    else if(mudblind==4){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 20/40");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }

    else if(mudblind==5){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 25/50");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }

    else if(mudblind==6){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 30/60");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    } 
 
    else if(mudblind==7){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 40/80");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
 
    else if(mudblind==8){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 50/100");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
 
    else if(mudblind==9){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 60/120");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
    
    else if(mudblind==10){
      lcd.setCursor(0,0);
      lcd.print("Blinds: 80/160");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
  
    else{
      lcd.setCursor(0,0);
      lcd.print("Blinds: 100/200");
      lcd.setCursor(0,1);
      lcd.print("Tempo Rest.:");
      imprimetemprest();
    }
}
  
void imprimetemprest(){ 
       
      lcd.setCursor(11,1);
      if(minutos>9)
        lcd.print(minutos);
      else{
        lcd.print("0");
        lcd.print(minutos);
      }
      lcd.setCursor(13,1);
      lcd.print(":");
      lcd.setCursor(14,1);
      if(segundos>9)
        lcd.print(segundos);
      else{
        lcd.print("0");
        lcd.print(segundos);
      }
    
}

void mux_enable(){
  
  if(VEZAT==0){
    digitalWrite(E2,LOW);
    digitalWrite(E1,LOW);
    digitalWrite(E0,LOW);
  }
  else if(VEZAT==1){
    digitalWrite(E2,LOW);
    digitalWrite(E1,LOW);
    digitalWrite(E0,HIGH);
  }
  else if(VEZAT==2){
    digitalWrite(E2,LOW);
    digitalWrite(E1,HIGH);
    digitalWrite(E0,LOW);
  }
  else if(VEZAT==3){
    digitalWrite(E2,LOW);
    digitalWrite(E1,HIGH);
    digitalWrite(E0,HIGH);
  }
  else if(VEZAT==4){
    digitalWrite(E2,HIGH);
    digitalWrite(E1,LOW);
    digitalWrite(E0,LOW);
  }
  else if(VEZAT==5){
    digitalWrite(E2,HIGH);
    digitalWrite(E1,LOW);
    digitalWrite(E0,HIGH);
  }
  else if(VEZAT==6){
    digitalWrite(E2,HIGH);
    digitalWrite(E1,HIGH);
    digitalWrite(E0,LOW);
  }
  else{
    digitalWrite(E2,HIGH);
    digitalWrite(E1,HIGH);
    digitalWrite(E0,HIGH);
  }

}     

void tempcont() { 
  
  if(sairint!=1){
    if(segundos==0&&minutos!=0){
      minutos--;
      segundos=59;
    }
    else
      segundos--;
  }
  
}
