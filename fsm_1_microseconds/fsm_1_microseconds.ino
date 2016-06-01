#include "TimerOne.h"

#define limite_luz 500 //valor entre 0 e 1023 para separar entre nivel alto e baixo
#define tamanho_dado 4 //tamanho do pacote
#define tamanho_pacote (2*tamanho_dado)
#define tempo 2000 //indica o tempo em us
#define PinLed 3

int a=0,recebidos=0,errados=0;      //contador do laço do transmissor de não enviar dados
unsigned long t1,t2,t1_inicial,t2_inicial;
volatile int stateT=0,stateR=0;
volatile int flag_envio=1;
volatile int countT,countR;
int pacoteT[tamanho_pacote];
int dadoT[tamanho_dado]={0,1,0,0};
int pacoteR[tamanho_pacote];
int dadoR[tamanho_dado];

void registra(){
  int conta=0;
  for(int i=0;i<tamanho_dado;i++){
    if(dadoR[i]==dadoT[i]){
      conta++;      
    }
  }
  if(conta==tamanho_dado){
    recebidos++;
  }else{
    errados++;
  }
  Serial.print("R ");
  Serial.print(recebidos);
  Serial.print(" E ");
  Serial.print(errados);
  Serial.print("\n");
}

void manchesterR(){
  int j=0;
  for(int i=0;i<tamanho_pacote;i=i+2){
    dadoR[j]=pacoteR[i];
    Serial.print(dadoR[j]);
    j++;
  }
  Serial.print("\n");
}

void manchesterT(){
  int j=0;
  for(int i=0; i<tamanho_pacote; i++){
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
    digitalWrite(PinLed,LOW);
    if((a==1)and(flag_envio)){
      noInterrupts();
      stateT=2;
      a=0;
      interrupts();
      break;
    }else{
      noInterrupts();
      stateT=0;
      a++;
      interrupts();
      break;
    }
    
    case 2:                       //envia 0, do bit 0
    digitalWrite(PinLed,LOW);
    noInterrupts();
    stateT=3;
    interrupts();
    break;

    case 3:                       //envia 1, do bit 0, e codifica o dado
    digitalWrite(PinLed,HIGH);
    manchesterT();
    noInterrupts();
    countT=0;
    stateT=4;
    interrupts();
    break;

    case 4:                     //envio do dado, apos retorna para o default
    digitalWrite(PinLed,pacoteT[countT]);
    noInterrupts();
    countT++;
    interrupts();
    if(countT>=tamanho_pacote){
      noInterrupts();
      stateT=0;
      interrupts();
    }
    break;
    
    default:                   //case 0, envia 1 do bit 1
    digitalWrite(PinLed,HIGH);
    noInterrupts();
    stateT=1;
    interrupts();
    break;   
  }
}

int recebe(){
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

void setup() {
  Serial.begin(9600);
  pinMode(2,OUTPUT);
  pinMode(PinLed,OUTPUT);
  Timer1.initialize(1000);
  Timer1.setPeriod(tempo);
  Timer1.attachInterrupt(transmissor);
}

void loop() {
  switch(stateR){  
    case 1:               //recebe o 0 do bit 1
    //Serial.println("B");
    while(1){
      if(recebe()==0){
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
      if(recebe()){
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
      if(recebe()==0){
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
      if(recebe()){
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
    pacoteR[countR]=recebe();
    noInterrupts();
    countR++;
    interrupts();
    if(countR>=tamanho_pacote){
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
      if(recebe())
      break;
    }      
     noInterrupts();
     stateR=1;
     t1=0;
     t2=0;
     interrupts();
     break;
  }
 }
