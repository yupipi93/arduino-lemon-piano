/* PIANO DE LIMONES V4
   Author : Yupipi93
   Date : 02/2019
   
   CODE 1: 6,5,6,7,2,5,2,1,3,4,X
   CODE 2: 3,6,1,4,2,5,3,6,1,4,X   
*/

#include "Notas.h"

//################################
//#########  MELODIAS ############
//################################

//MELODIA GAME OVER
const int death[] = {17, NOTE_C4, 32, NOTE_CS4, 32, NOTE_D4, 16, NOTE_H, 4, NOTE_H, 2, NOTE_B3, 8, NOTE_F4, 8, NOTE_H, 8, NOTE_F4, 8, NOTE_F4, 6, NOTE_E4, 6, NOTE_D4, 6, NOTE_C4, 8, NOTE_E3, 8, NOTE_H, 8, NOTE_E3, 8, NOTE_C3, 8};



//MAIN THEME NOTAS (MELODIA COMPLETA)
const int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};

//MAIN THEME TEMPOS (MELODIA COMPLETA)
const int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

//MAIN THEME NOTAS RECORTADO (LO QUE SUENA AL COMPLETAR EL JUEGO)
const int melody_cut[] = {//-7 columnas
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};

//MAIN THEME TEMPOS RECORTADO (LO QUE SUENA AL COMPLETAR EL JUEGO)
const int tempo_cut[] = {//-7 columnas
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

//UNTERWORLD NOTAS (MELODIA COPMPLETA) 
const int underworld_melody[] = {
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};

//UNTERWORLD TEMPOS (MELODIA COPMPLETA) 
const int underworld_tempo[] = {
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

//UNTERWORLD NOTAS RECORTADO (LO QUE SUENA AL COMPLETAR EL JUEGO)
const int underworld_melody_cut[] = {//-4filas
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};

//UNTERWORLD TEMPOS RECORTADO (LO QUE SUENA AL COMPLETAR EL JUEGO)
const int underworld_tempo_cut[] = {//-4filas
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

//################################
//###########  PINES #############
//################################
#define Buzz 8
#define LedRojo 2
#define LedVerde 3
#define CambioJuego 4
#define Relee_1 5
#define Relee_2 6

//################################
//#########  CONSTANTES ##########
//################################
const int MaxFallos = 10;//Numero de penalizaciones permitidas (evita llenar la botella de agua demasiado)
const int Sensibilidad = 100; //Aumentar valor si suena sin tocar los limones. (170 para portatil con cargador)
const int Duracion = 5çkp`ñ´sca
`ç´fas0; //Duracion de la nota, se traduce como sensacion de tiempo real(cuanto menos, mejor sensacion)
const bool serial = true;

//################################
//#########  VARIABLES ###########
//################################

int tamSecuencia = 10;  //NUMERO DE NOTAS QUE HAY QUE ADIVINAR (MAX 10)
int numNota = -1;       //INDICE DEL VECTOR DE LA SECUENCIA(AVANZA CON CADA NOTA CORRECTA, SE REINICIA SI FALLAS)
int notaPulsada = 0;    //ULTIMA NOTA PULSADA
int juego;//(CAMBIAR 1,2 POR 0 Y 7)     //ELECTOR DEL JUEGO EN EL QUE ESTAMOS (1 = MAIN MARIO THEME, 2 = UNDERGROUND MARIO THEME) 
int selectorTeclado;    //SI VALE 0 SELECCIONA LA PRIMERA PARTE DEL TECLADO PARA EL JUEGO 1, VALE 7 PARA JUEGO 2
int notaSecuencia;      //NOTA DE LA SECUENCIA CORRESPONDIENTE
bool start = false;     //CODIGO QUE SOLO SE EJECUTARA SI ESTAR ES FALSE
int fallos = 0;         //NUMERO DE PENALIZACIONES EFECTUADAS
bool muerte = true;     //BOOLEAN PARA CUANDO ALCANZAS NUMERO MAXIMO DE PENALIZACIONES SUENE MELODIA SI TOCAS NOTA
int count = 0;          //CONTADOR DE VUELTAS DEL LOOP

//SECUENCIA SECRETA 1
int secuencia_1 [10] = {NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_G6, NOTE_C7, NOTE_G6, NOTE_E6, NOTE_A6, NOTE_B6};

//SECUENCIA SECRETA 2
int secuencia_2 [10] = {NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_AS3, NOTE_AS4, NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4};

//NOTAS de la 0 a la 6 para juego 1 y de la 7 a la 13 para juego 2
int tecla[14] = {NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, NOTE_E7, NOTE_G7, NOTE_A3, NOTE_AS3, NOTE_C4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5};




//################################
//###########  SETUP #############
//################################
void setup() {  
   // Serial.begin(9600); 
  
  pinMode(0,INPUT);
  pinMode(1,INPUT);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(6,INPUT); 
  pinMode(7,INPUT);     
  pinMode(Buzz, OUTPUT);
  pinMode(LedRojo, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(CambioJuego, INPUT);
  pinMode(Relee_1, OUTPUT);
  pinMode(Relee_2, OUTPUT);
  

  
}


//################################
//###########  BUCLE #############
//################################
void loop() { 
  //LECTURA POR EL MONITOR
   /*   
      for(int i = 0; i<7; i++){
        delay(120);      
        Serial.print("Analog ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(analogRead(i));
      }
    */
 if(digitalRead(7) == HIGH){
  start = false;
 }


  //ELEJIR JUEGO AL INICIO DEL PROGRAMA (NECESARIO MEJORAR)
  if (!start) {//
      analogWrite(Relee_1, HIGH);
      analogWrite(Relee_2, HIGH);
    start = true;
    if (digitalRead(CambioJuego) == HIGH) {
      juego = 1;
      numNota = -1;
      notaPulsada  = 0;
    } else {
      juego = 2;
      numNota = -1;
      notaPulsada  = 0;
    }
  }

/*
  if(analogRead(0)>Sensibilidad){
    if(analogRead(1)>Sensibilidad){
      if(juego = 1){
          juego = 7;
      }else{
        juego =1;
      }
    }    
  }
  */

    //SELEECCION DEL TECLADO SEGUN EL JUEGO SELECCIONADO (mejorar)
  if (juego > 0) {
    if (juego == 1) {
      selectorTeclado = 0;
    } else {
      selectorTeclado = 7;
    }

//################################
//#######  LECTURA DATA ##########
//################################



for(int i = 0; i<7 ;i++){
   if (analogRead(i)>Sensibilidad) {
        tone(Buzz, tecla[i + selectorTeclado], Duracion);
        if (notaPulsada != tecla[i + selectorTeclado]) {
          //Relee_2 = 8;//probar fallos
          numNota++;
        }
        notaPulsada = tecla[i + selectorTeclado];
        count = 0;        
        muerte = true;
      }
}

//################################
//#####  COMPROBACIONES ##########
//################################


/*si nota que es la ultima nota pulsada es igual a la posicion de la secuenta,
      se enciende verde 5 segundos, cada vez que pulsamos una nota aumenta 1 la secuencia
      si la nnta coincide con secuencia, enciende rojjo y reinicia secuencia
    */

    if (juego == 3 && muerte) {
      death_melody();
      muerte = false;
    } else {


      //si acietas las 10 notas segidas suena la cacion completa
      if (numNota >= tamSecuencia && juego != 3) {
        numNota = 0;

        if (juego == 1) {
          //Juego1
          sing(-1);//continuacion cancion 1 recortada
          // sing(1);//cancion1 entera
        }
        if (juego == 2) {
          //Juego2
          sing(-2);//continuacion cancion 2 recortada
          //S sing(2);//cancion1 entera
        }
        fallos = 0;


      }

      //si nota no tiene el valor inicial, comprueba que nota(tecla pulsada) coincida con la poscion de la secuncia secreta
      if (notaPulsada != 0) {
        if (juego == 1) {
          notaSecuencia = secuencia_1[numNota];
        } else {
          notaSecuencia = secuencia_2[numNota];
        }

        //ACIERTAS
        if (notaPulsada == notaSecuencia) {
          //digitalWrite(LedRojo, LOW);
          digitalWrite(LedVerde, HIGH);

          //FALLAS
        } else {
          digitalWrite(LedVerde, LOW);
          digitalWrite(LedRojo, HIGH);
          //si fallas a partir de la nota 7 activar bomba de agua
          if (numNota >= 7) {
            digitalWrite(Relee_1, HIGH);
            digitalWrite(Relee_2, LOW);
            tone(Buzz, NOTE_D1);
            delay(1000);
            noTone(Buzz);
            digitalWrite(Relee_1, LOW);
            digitalWrite(Relee_2, HIGH);            
            fallos++;
            if (fallos >= MaxFallos) {
              juego = 3;
              muerte = true;
            }

          }//nota >=7
          //reinicias
          numNota = -1;
          notaPulsada = 0;
        }//si no nota = nota Secuencia

        // apaga leds tras 5 segundos
        count++;
        if (count >= 50) {
          //nota = 0;
          digitalWrite(LedVerde, LOW);
          digitalWrite(LedRojo, LOW);
          count = 0;
        }


      }
    }//juegono 3 o !muerte
  }//juego no > 0


}//loop
//################################
//##########  FUNCIONES ##########
//################################


int song = 0;

void sing(int s) {
  // iterate over the notes of the melody:
  song = s;
  if (song == 2) {
    Serial.println(" 'Underworld Theme'");
    int size = sizeof(underworld_melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / underworld_tempo[thisNote];

      buzz(Buzz, underworld_melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(Buzz, 0, noteDuration);

    }

  } else if (song == -2) {
    Serial.println(" 'Underworld Theme CUT'");
    int size = sizeof(underworld_melody_cut) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / underworld_tempo_cut[thisNote];

      buzz(Buzz, underworld_melody_cut[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(Buzz, 0, noteDuration);

    }

  } else if (song == 1) {

    Serial.println(" 'Mario Theme'");
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / tempo[thisNote];

      buzz(Buzz, melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(Buzz, 0, noteDuration);

    }
  } else if (song == -1) {

    Serial.println(" 'Mario Theme CUT'");
    int size = sizeof(melody_cut) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / tempo_cut[thisNote];

      buzz(Buzz, melody_cut[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(Buzz, 0, noteDuration);

    }
  }
}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(13, LOW);

}





void death_melody() {
  for (int thisNote = 1; thisNote < (death[0] * 2 + 1); thisNote = thisNote + 2) {
    tone(Buzz, death[thisNote], (1000 / death[thisNote + 1]));
    delay((1000 / death[thisNote + 1]) * 1.30);
    noTone(Buzz);
  }
}
  
