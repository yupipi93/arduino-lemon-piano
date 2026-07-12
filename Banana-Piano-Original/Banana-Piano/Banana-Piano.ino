
/*notas musicales*/
#include "pitches.h"


// DECLARACION DE VARIABLES PARA PINES
const int pinecho = 11;
const int pintrigger = 12;
const int pinled = 13;
 
// VARIABLES PARA CALCULOS
unsigned int tiempo, distancia;
 
//SALIDA TONO
const int Buzz = 8;


void setup(){
  // PREPARAR LA COMUNICACION SERIAL
  //Serial.begin(9600);
  // CONFIGURAR PINES DE ENTRADA Y SALIDA
  pinMode(pinecho, INPUT);
  pinMode(pintrigger, OUTPUT);
  pinMode(13, OUTPUT);
  
// Declaro el zumbador
  pinMode(Buzz, OUTPUT);

}

void loop(){

  /*
 // ENVIAR PULSO DE DISPARO EN EL PIN "TRIGGER"
  digitalWrite(pintrigger, LOW);
  //delayMicroseconds(2);
  digitalWrite(pintrigger, HIGH);
  // EL PULSO DURA AL MENOS 10 uS EN ESTADO ALTO
  //delayMicroseconds(10);
  digitalWrite(pintrigger, LOW);
 
  // MEDIR EL TIEMPO EN ESTADO ALTO DEL PIN "ECHO" EL PULSO ES PROPORCIONAL A LA DISTANCIA MEDIDA
  tiempo = pulseIn(pinecho, HIGH);
 
  // LA VELOCIDAD DEL SONIDO ES DE 340 M/S O 29 MICROSEGUNDOS POR CENTIMETRO
  // DIVIDIMOS EL TIEMPO DEL PULSO ENTRE 58, TIEMPO QUE TARDA RECORRER IDA Y VUELTA UN CENTIMETRO LA ONDA SONORA
  distancia = tiempo / 58;
 
  // ENVIAR EL RESULTADO AL MONITOR SERIAL
  //Serial.print(distancia);
  //Serial.println(" cm");
  //delay(200);
 */


  


//TECLADO LIMONES
 if( ((analogRead(0)+analogRead(0))/2) <=1019){
  tone(Buzz, NOTE_C3, 100);    
 }

 if( ((analogRead(1)+analogRead(1))/2)<= 1019){  
   tone(Buzz, NOTE_D3, 100);
 }

if( ((analogRead(2)+analogRead(2))/2)<= 1019){  
   tone(Buzz, NOTE_E3, 100);
 }

if( ((analogRead(3)+analogRead(3))/2)<= 1019){  
   tone(Buzz, NOTE_F3, 100);
 }

if( ((analogRead(4)+analogRead(4))/2)<= 1019){  
   tone(Buzz, NOTE_G3, 100);
 }

if( ((analogRead(5)+analogRead(5))/2)<= 1019){  
   tone(Buzz, NOTE_A3, 100);
 }

if( ((analogRead(6)+analogRead(6))/2)<= 1019){  
   tone(Buzz, NOTE_B3, 150);
 }
 
 






}
