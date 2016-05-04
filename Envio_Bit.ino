#include "Timer.h"

#define on 1023   //brilho do led em nivel alto
#define off 0   //brilho do led em nivel baixo
#define limite_luz 500 //valor entre 0 e 1023 para separar entre nivel alto e baixo
#define tamanho_pacote 8 //tamanho do pacote
#define tempo 5 //indica o tempo entre o chaveamento

Timer *timerT = new Timer(1);
Timer *timerR = new Timer(1);
Timer *timer = new Timer(5);

int count;
int teste;

int recebe(){
  int pacote;
  digitalWrite(2,HIGH);
  pacote=analogRead(A0);
  digitalWrite(2,LOW);
  return pacote;
}

int recebeBit(){
  int pacote;
  pacote=recebe();
  if(pacote>=limite_luz)
    pacote=1;
  else
    pacote=0;
  return pacote;
}
  
void enviaBit(int dado){
    if(dado==1)
    dado=on;
    else
    dado=off;
    digitalWrite(3,dado);
}

void Transmissor(){
   teste=not teste;
   Serial.println("T");
   enviaBit(teste);
   Serial.println(teste);
}

void Controle(){
  noInterrupts();
  count++;
  interrupts();
  if(count==1)
  timerT->Update();
  if(count==2)
  timerR->Update();
  if(count==1000){
   noInterrupts();
   count=0;
   interrupts();
}
}

void Receptor(){
  int dado;
  Serial.println("R");
  dado=recebeBit();
  digitalWrite(3,HIGH);     //acende led   
  Serial.println(dado);
}


void setup() {
  Serial.begin(9600);       //comunicação serial
  pinMode(2,OUTPUT);        // alimentação fotodiodo
  pinMode(3,OUTPUT);        // led
  digitalWrite(3,HIGH);     //acende led
  timerT->setOnTimer(&Transmissor);   //interrupção para transmitir
  timerR->setOnTimer(&Receptor);      // interrupção para receber
  timer->setOnTimer(&Controle);          //interrupção para ativar o envio(manter a luz ligada)
  timerT->Start();                    //dispara timer transmissor
  timerR->Start();                    //dispara timer receptor
  timer->Start();                     //dispara o timer geral
  count=0;
  teste=0;
}

void loop() {
  timer->Update();  
}
