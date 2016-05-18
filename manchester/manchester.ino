#include "TimerOne.h"

#define limite_luz 500 //valor entre 0 e 1023 para separar entre nivel alto e baixo
#define tamanho_pacote 4 //tamanho do pacote
#define tamanho 8
#define tempo 2 //indica o tempo em ms
#define tempoUs 2000    //mesmo tempo em us
int atual=0,anterior=0,count=0;
int x[tamanho_pacote]={0,0,0,1};



void recebePacote(){         //armazena em um vetor os valores recebidos na entrada analogica
  int pacote[tamanho],j=0;
  for(int i=0; i<tamanho; i++){ //recebe os bits
    pacote[i]=recebeBit();
    delay(tempo);
  }
  for(int i=0; i<tamanho; i=i+2){  //verifica se o primeiro bit recebido é 1 ou 0 e descarta o proximo
    if(pacote[i]>pacote[i+1])
    pacote[j]=1;
    else
    pacote[j]=0;
    j++;
  }
   for(int i=0; i<tamanho_pacote; i++){   //mostra o pacote recebido
     Serial.print(pacote[i]);
   }
  Serial.println();
}

int recebeBit(){
  int dado;
  digitalWrite(2,HIGH);
  dado=analogRead(A0);
 // dado=dado+analogRead(A0);
 // dado=dado+analogRead(A0);
  digitalWrite(2,LOW);
//  dado=dado/2;
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
    digitalWrite(3,dado);
}

void desligaLed(){
  digitalWrite(3,LOW);
}

void enviaPacote(){
  noInterrupts();
  count++;
  interrupts();
  if(count==1){                           //desliga o led por um pulso para iniciar a transmissão
    desligaLed();
  }
  if((count>1)&&(count<10)&&(count%2==0)){  //envia o bit correspondente no x, de 0 a 4
    enviaBit(x[(count/2)-1]);
  }
  if((count>1)&&(count<10)&&(count%2!=0)){  //envia o complementar do ultimo enviado
    enviaBit(not(x[(count/2)-1]));
  }
  if(count==10){                          //apos o envio liga o led novamente
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
  Timer1.initialize(0);
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
