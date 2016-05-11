#include "TimerOne.h"

#define limite_luz 500 //valor entre 0 e 1023 para separar entre nivel alto e baixo
#define tamanho_pacote 8 //tamanho do pacote
#define tempo 4 //indica o tempo em ms
#define tempoUs 4000    //mesmo tempo em us
int atual,anterior,count;
int x[tamanho_pacote]={1,0,1,0,1,0,1,0};



void recebePacote(){         //armazena em um vetor os valores recebidos na entrada analogica
  int pacote[tamanho_pacote];
  for(int i=0; i<tamanho_pacote; i++){
    pacote[i]=recebeBit();
    delay(tempo);
  }
  Serial.println("Pacote Recebido");
  for(int i=0; i<tamanho_pacote; i++){
    Serial.print(pacote[i]);
  }
  Serial.println();
}

int recebeBit(){
  int dado;
  digitalWrite(2,HIGH);
  dado=analogRead(A0);
  dado=dado+analogRead(A0);
  dado=dado+analogRead(A0);
  digitalWrite(2,LOW);
  dado=floor(dado/3);
  if(dado>=limite_luz)
    dado=1;
  else
    dado=0;
  return dado;
}

void ligaLed(){
  digitalWrite(3,HIGH);
}

void enviaBit(int dado){
    digitalWrite(3,x[dado]);
}

void transmite(){
  digitalWrite(3,LOW);
}

void enviaPacote(){
  noInterrupts();
  count++;
  interrupts();
  if(count==(99-tamanho_pacote)){
    transmite();
  }
  if((count>=(100-tamanho_pacote))&&(count<100)){
    enviaBit(count-(100-tamanho_pacote));
  }
  if(count==100){
   noInterrupts();
   count=0;
   interrupts();
   ligaLed();
}
}

void setup() {
  Serial.begin(9600);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  Timer1.initialize(10);
  Timer1.setPeriod(tempoUs);
  Timer1.attachInterrupt(enviaPacote);
  ligaLed();
}

void loop() {
  noInterrupts();
  atual=recebeBit();
  interrupts();
  if(atual==0 && anterior==1){       //queda, incio de recepção 
    delay(tempo);
    recebePacote();
    noInterrupts();
    anterior=atual;
    interrupts();
  }
  else{
    noInterrupts();
    anterior=atual;
    interrupts();
  }
}
