#include "TimerOne.h"

#define LIMITE_LUZ 3  //valor entre 0 e 1023 para separar entre nivel alto e baixa
#define TAMANHO_DADO 4 //tamanho do pacote
#define TAMANHO_PACOTE (2*TAMANHO_DADO)
#define TEMPO 2000 //indica o tempo em us
#define PIN_LED 3

int limite_luz=500,media=0,repete=0,total=0,certos=0,errados=0;    //CALIBRAÇÃO, adicionei media e limite_luz  //contador do laço do transmissor de não enviar dados
unsigned long t1,t2,t1_inicial,t2_inicial;
volatile int stateT=0,stateR=0;
volatile int flag_envio=1;
volatile int countT,countR;
int pacoteT[TAMANHO_PACOTE];
int dadoT[TAMANHO_DADO]={1,0,1,0};
int pacoteR[TAMANHO_PACOTE];
int dadoR[TAMANHO_DADO];

void registra(){
  int conta=0;
  total++;
  noInterrupts();
  flag_envio=0;
  interrupts();
  for(int i=0;i<TAMANHO_DADO;i++){
    if(dadoR[i]==dadoT[i]){
      noInterrupts();
      conta++;      
      interrupts();
    }
  }
  if(conta==TAMANHO_DADO){
    noInterrupts();
    certos++;
    interrupts();
  }else{
    noInterrupts();
    errados++;
    interrupts();
  }
 // if(total==1000){
  Serial.print("T ");
  Serial.print(total);
  Serial.print(" C ");
  Serial.print(certos);
  Serial.print(" E ");
  Serial.print(errados);
  Serial.print("\n");
 // }
  noInterrupts();
  flag_envio=1;
  interrupts();
}

void manchesterR(){
  int j=0;
  for(int i=0;i<TAMANHO_PACOTE;i=i+2){
    dadoR[j]=pacoteR[i];
    Serial.print(dadoR[j]);
    j++;
  }
  Serial.print("\n");
}

void manchesterT(){
  int j=0;
  for(int i=0; i<TAMANHO_PACOTE; i++){
    if(i%2==0){
    pacoteT[i]=dadoT[j];
    }else{
    pacoteT[i]=not(dadoT[j]);
    j++;
    }
  }
}

void transmissor(){
  switch(stateT){    
    case 1:                     // envia 0 do bit 1, se tiver dado pra enviar vai pro 2, se não volta pro 1(Default que manda 1 do bit 1)
    digitalWrite(PIN_LED,LOW);
    if((repete==1)and(flag_envio)){
      noInterrupts();
      stateT=2;
      repete=0;
      interrupts();
      break;
    }else{
      noInterrupts();
      stateT=0;
      repete++;
      interrupts();
      break;
    }
    
    case 2:                       //envia 0, do bit 0
    digitalWrite(PIN_LED,LOW);
    noInterrupts();
    stateT=3;
    interrupts();
    break;

    case 3:                       //envia 1, do bit 0, e codifica o dado
    digitalWrite(PIN_LED,HIGH);
    manchesterT();
    noInterrupts();
    countT=0;
    stateT=4;
    interrupts();
    break;

    case 4:                     //envio do dado, apos retorna para o default
    digitalWrite(PIN_LED,pacoteT[countT]);
    noInterrupts();
    countT++;
    interrupts();
    if(countT>=TAMANHO_PACOTE){
      noInterrupts();
      stateT=0;
      interrupts();
    }
    break;
    
    default:                   //case 0, envia 1 do bit 1
    digitalWrite(PIN_LED,HIGH);
    noInterrupts();
    stateT=1;
    interrupts();
    break;   
  }
}

int recebeSimbolo(){
  int x;
  digitalWrite(2,HIGH);
  x=analogRead(A0);
  x=x+analogRead(A0);
 // dado=dado+analogRead(A0);
  digitalWrite(2,LOW);
//  dado=dado/2;
  if(x>=2*limite_luz)
    x=1;
  else
    x=0;
  return x;
}

int recebe(){
  int x;
  digitalWrite(2,HIGH);
  x=analogRead(A0);
  x=x+analogRead(A0);
  x=x>>1;
  return x;
}

void setup() {
  Serial.begin(230400);
  pinMode(2,OUTPUT);
  pinMode(PIN_LED,OUTPUT);
  Timer1.initialize(0);
  Timer1.setPeriod(TEMPO);
  Timer1.attachInterrupt(transmissor);
}

void loop() {
  switch(stateR){  
    case 1:               //recebe o 0 do bit 1
//    Serial.println("B");
    while(1){
      if(recebeSimbolo()==0){
        limite_luz=(LIMITE_LUZ+((media+recebe())>>1));  //CALIBRAÇÃO, define o limite de luz como a media entre o nivel alto no case 0 e nivel baixo no case 1 e soma com o "ganho" escolhido 
        t1_inicial=micros();
        break;
      }
    }
    noInterrupts();
    stateR=2;
    interrupts();
    break;

    case 2:           //recebe o 1 do bit 1
    //Serial.println("C");
    while(1){
      if(recebeSimbolo()){
        t1=micros()-t1_inicial;
        break;
      }
    }
    noInterrupts();
    stateR=3;
    interrupts();
    break;
   
    case 3:         //recebe o 0 do bit 0
    //Serial.println("D");
    while(1){
      if(recebeSimbolo()==0){
        t2_inicial=micros();
        break;
      }
    }
    noInterrupts();
    stateR=4;
    interrupts();
    break;

    case 4:
    //Serial.println("E");    //recebe o 1 do bit 0
    while(1){
      if(recebeSimbolo()){
         t2=micros()-t2_inicial;
         break;
      }
    }
    if(t2>(1.5*t1)){
    noInterrupts();
    stateR=5;
    countR=0;
    interrupts();
    break;
    }else{
    noInterrupts();
    stateR=0;
    interrupts();
    break;
    }

    case 5:
    //Serial.println("F");
    delayMicroseconds(t1);
    pacoteR[countR]=recebeSimbolo();
    noInterrupts();
    countR++;
    interrupts();
    if(countR>=TAMANHO_PACOTE){
      manchesterR();
      registra();
      noInterrupts();
      stateR=0;
      interrupts();
     }
     break;

    default:                  //aguarda o simbolo 1 do bit 1
    //Serial.println("A");
    while(1){
      if(recebeSimbolo()){
        media=recebe();     //CALIBRAÇÃO
        break;
      }
    }      
     noInterrupts();
     stateR=1;
     t1=0;
     t2=0;
     interrupts();
     break;
  }
 }
