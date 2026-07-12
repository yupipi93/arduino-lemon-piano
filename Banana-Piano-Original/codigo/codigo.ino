#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
 
#define Buzz 8


/*
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
 */

//NOTE_E7 NOTE_C7 NOTE_G7 NOTE_G6 NOTE_E6 NOTE_A6 NOTE_B6
//Mario main theme melody
int melody_cut[] = {//-7 columnas
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

int tempo_cut[] = {//-7 columnas
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


//Mario main theme melody
int melody[] = {
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
//Mario main them tempo
int tempo[] = {
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
//Underworld melody
int underworld_melody[] = {
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
//Underwolrd tempo
int underworld_tempo[] = {
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



int underworld_melody_cut[] = {//-4filas
  
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
//Underwolrd tempo
int underworld_tempo_cut[] = {//-4filas
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
 


//PINES
const int LedRojo = 2;
const int LedVerde = 3;
const int botonJuego1 = 4;
const int relee = 5;


//CONTADOR DE TIEMPO
int count;


  //SECUENCIA 1  
  int secuencia_1 [10]= {NOTE_E7,NOTE_C7,NOTE_E7,NOTE_G7,NOTE_G6,NOTE_C7,NOTE_G6,NOTE_E6,NOTE_A6,NOTE_B6};
  //NOTAS de la 0 a la 6 para juego 1 y de la 7 a la 13 para juego 2
  int tecla[14] = {NOTE_E6,NOTE_G6,NOTE_A6,NOTE_B6,NOTE_C7,NOTE_E7,NOTE_G7,NOTE_A3,NOTE_AS3,NOTE_C4,NOTE_A4,NOTE_AS4,NOTE_C5,NOTE_D5}; 

  //SECUENCIA 2
  int secuencia_2 [10]= {NOTE_C4,NOTE_C5,NOTE_A3,NOTE_A4,NOTE_AS3,NOTE_AS4,NOTE_C4,NOTE_C5,NOTE_A3,NOTE_A4};  
  //NOTAS 2
  //int teclas_2[7] = {NOTE_A3,NOTE_AS3,NOTE_C4,NOTE_A4,NOTE_AS4,NOTE_C5,NOTE_D5};
 



int tamSecuencia = 9;//tam secuencia -1
int numNota;
int nota;
int juego;
int selectorTeclado;
int notaSecuencia;
bool start;

void setup(){
 //Serial.begin(9600);
// Declaro el zumbador
  pinMode(Buzz, OUTPUT);
  pinMode(LedRojo, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(botonJuego1, INPUT);
  pinMode(relee, OUTPUT);  
  count = 0;
  numNota = -1;
  nota  = 0;
  
  start = false;
  

}

void loop(){
/*
  delay(10);
 Serial.print("Analog 0: ");
 Serial.println(analogRead(0));
 Serial.print("Analog 0: ");
 Serial.println(analogRead(0));
 Serial.print("Analog 0: ");
 Serial.println(analogRead(0));
 Serial.print("Analog 0: ");
 Serial.println(analogRead(0));
 Serial.println(" ");
 
 Serial.print("Analog 1: ");
 Serial.println(analogRead(1));
 Serial.print("Analog 1: ");
 Serial.println(analogRead(1));
 Serial.print("Analog 1: ");
 Serial.println(analogRead(1));
 Serial.print("Analog 1: ");
 Serial.println(analogRead(1));
 Serial.println(" ");
 
*/
/*
digitalWrite(relee, HIGH);
delay(300);
digitalWrite(relee, LOW);
*/


//ELEJIR JUEGO
if(!start){
  start = true;
  if(digitalRead(botonJuego1)== HIGH){
    juego = 1;
    numNota = -1;
    nota  = 0;
  }else{
    juego = 2;
    numNota = -1;
    nota  = 0;
  }
}




if(juego > 0){
  if(juego == 1){
    selectorTeclado = 0;
  }else{
     selectorTeclado = 7;
  }

//################################
//#######  LECTURA DATA ##########
//################################
  
  if(juego)
  //Teclado
   if( ((analogRead(0)+analogRead(0)+analogRead(0)+analogRead(0))/4) <=1019){
    tone(Buzz, tecla[0+selectorTeclado], 150);    
    count = 0;
    if(nota != tecla[0+selectorTeclado]){
      numNota++; 
    }  
    nota = tecla[0+selectorTeclado];
   }
  
   if( ((analogRead(1)+analogRead(1)+analogRead(1)+analogRead(1))/4)<= 1019){  
     tone(Buzz, tecla[1+selectorTeclado], 150);   
      count = 0;  
      if(nota != tecla[1+selectorTeclado]){
      numNota++; 
    }
      nota = tecla[1+selectorTeclado];
   }
  
  if( ((analogRead(2)+analogRead(2)+analogRead(2)+analogRead(2))/4)<= 1019){  
     tone(Buzz, tecla[2+selectorTeclado], 150);
     count = 0;
     if(nota != tecla[2+selectorTeclado]){
      numNota++; 
    }
     nota = tecla[2+selectorTeclado];
   }
  
  if( ((analogRead(3)+analogRead(3)+analogRead(3)+analogRead(3))/4)<= 1019){  
     tone(Buzz, tecla[3+selectorTeclado], 150);
     count = 0;
     if(nota != tecla[3+selectorTeclado]){
      numNota++; 
    }
    nota = tecla[3+selectorTeclado];
   }
  
  if( ((analogRead(4)+analogRead(4)+analogRead(4)+analogRead(4))/4)<= 1019){  
     tone(Buzz, tecla[4+selectorTeclado], 150);
     count = 0;
     if(nota != tecla[4+selectorTeclado]){
      numNota++; 
    }
     nota = tecla[4+selectorTeclado];
   }
  
  if( ((analogRead(5)+analogRead(5)+analogRead(5)+analogRead(5))/4)<= 1019){  
     tone(Buzz, tecla[5+selectorTeclado], 150);
     count = 0;
     if(nota != tecla[5+selectorTeclado]){
      numNota++; 
    }
     nota = tecla[5+selectorTeclado];
   }
  
  if( ((analogRead(6)+analogRead(6)+analogRead(6)+analogRead(6))/4)<= 1019){  
     tone(Buzz, tecla[6+selectorTeclado], 150);
     count = 0;
     if(nota != tecla[6+selectorTeclado]){
      numNota++; 
    }
     nota = tecla[6+selectorTeclado];
   }
//################################
//#####  COMPROBACIONES ##########
//################################
  
  /*si nota que es la ultima nota pulsada es igual a la posicion de la secuenta, 
  *se enciende verde 5 segundos, cada vez que pulsamos una nota aumenta 1 la secuencia
  *si la nnta coincide con secuencia, enciende rojjo y reinicia secuencia
  */


  //si acietas las 10 notas segidas suena la cacion completa
  if(numNota >= tamSecuencia) {
    numNota = 0;
  
    if(juego == 1){
      //Juego1
      sing(-1);//continuacion cancion 1 recortada
     // sing(1);//cancion1 entera
    }else{
      //Juego2
      sing(-2);//continuacion cancion 2 recortada
     //S sing(2);//cancion1 entera
    }
    
    
  }

  //si nota no tiene el valor inicial, comprueba que nota(tecla pulsada) coincida con la poscion de la secuncia secreta
  if(nota != 0){
    if(juego == 1){
      notaSecuencia = secuencia_1[numNota];
    }else{
      notaSecuencia = secuencia_2[numNota];
    }

    //ACIERTAS
    if(nota == notaSecuencia){
      //digitalWrite(LedRojo, LOW);
      digitalWrite(LedVerde, HIGH);
      
    //FALLAS  
    }else{
      digitalWrite(LedVerde, LOW);
      digitalWrite(LedRojo, HIGH);
      numNota = -1;
      nota = 0;
    }
  }
  
  // apaga leds tras 5 segundos
   count++;
   if(count >= 20){
    //nota = 0;
    digitalWrite(LedVerde, LOW); 
    digitalWrite(LedRojo, LOW); 
    count = 0;
   }
  
  
  }
}
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
 
  }else if (song == -2) {
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
 
  }else if(song == 1){
 
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
  }else if(song == -1){
 
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
