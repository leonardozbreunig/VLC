#include "TimerOne.h"

#define on 1023   //brilho do led em nivel alto
#define off 0 //brilho do led em nivel baixo
#define limite_luz 500 //valor entre 0 e 1023 para separar entre nivel alto e baixo
#define tamanho_pacote 8 //tamanho do pacote
#define tempo 5 //indica o tempo entre o chaveamento

int atual,anterior,count;
int x[tamanho_pacote]={1,1,0,0,0,0,0,0};



void recebePacote(){         //armazena em um vetor os valores recebidos na entrada analogica
  int pacote[tamanho_pacote];
  for(int i=0; i<tamanho_pacote; i++){
    pacote[i]=recebeBit();
    Serial.println("Recebendo");
    Serial.println(pacote[i]);;
    delay(tempo);
  }
}

int recebeBit(){
  int dado;
  digitalWrite(2,HIGH);
  dado=analogRead(A0);
  delay(tempo);
  digitalWrite(2,LOW);
  ligaLed();
  if(dado>=limite_luz)
    dado=1;
  else
    dado=0;
  return dado;
}

void ligaLed(){
  digitalWrite(3,on);
}

void enviaBit(int dado){
    digitalWrite(3,x[dado]);
    delay(tempo);
}

void transmite(){
  digitalWrite(3,off);
  delay(tempo);
}

void enviaPacote(){
  noInterrupts();
  count++;
  interrupts();
  if(count==1){
    transmite();
    geraPacote();
  }
  if((count>=2)&&(count<10)){
    enviaBit(count-2);
  }
  if(count==1000){
   noInterrupts();
   count=0;
   interrupts();
}
}

void geraPacote(){
  for(int i=0; i<tamanho_pacote;i++){
    if(x[i]==1)
    x[i]=on;
    else
    x[i]=off;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  Timer1.initialize(100);
  Timer1.attachInterrupt(enviaPacote);
  ligaLed();
}

void loop() {
  noInterrupts();
  atual=recebeBit();
  interrupts();
  if(atual==0 && anterior==1){       //queda, incio de recepção 
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
