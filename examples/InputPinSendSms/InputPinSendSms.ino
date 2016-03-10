/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: InputPinSendSms
 
   Este exemplo mostra ira enviar um SMS para o numero cadastrado sempre que o pino INPUT_PIN
   for acionado
   
*/

#include <SoftwareSerial.h>
#include <StringParser.h>
#include <Timer.h>

#define   DEBUG
#ifdef  DEBUG
  #define IF_DEBUG(x) x;
#else
  #define IF_DEBUG(x)
#endif

#define INPUT_PIN    2                            // Pino que se deseja verificar o input
#define MINUTES_FAIL 5                            // Minutos sem REDE necessario para resetar o SIM900
#define NUMBER_SMS   "8599999999"                 // Numero ao qual sera enviada se o pino 2 for acionado
#define MESSAGE_SMS  "Mensagem a ser enviada"     // Mensagem a ser enviada

#include <freedomboard.h>
#include <freedomsim900.h>

Timer          timer;               // Checar tarefas periodicamente
StringParser   buffer(64);          // Parser para extrair palavras
String         number;              // Ultimo numero que recebeu SMS

SoftwareSerial SoftSerial(8,7);     // Comunicacao entre FreedomBoard e SIM900

FreedomBoard   fb;                  // Freedom Board
FreedomSim900  SIM900(SoftSerial);  // Modulo SIM900

bool           hasNetwork = false;    // retorno de AT+CPAS (0 - hasNetwork)
uint8_t        countFail  = 0;


/* ----------------------------------------------------------------------------- */

void checkNetwork() {
 
   // AT+CPAS (Deve retornar 0)
   int i = SIM900.activityStatus();
   #ifdef DEBUG
   Serial.print("activityStatus: "); Serial.println(i);
   #endif
   
   hasNetwork = (i == 0);
   
   // Reseta o SIM900 se fica mais de 5 minutos sem rede
   // Esta rotina e' chamada a cada 10 segundos
   
   if(!hasNetwork) {
     
     if(++countFail > MINUTES_FAIL * 60 / 10) { // 5 Minutes  
       SIM900.reset();
       countFail = 0;
     }
   }
}

/* ----------------------------------------------------------------------------- */

#ifdef DEBUG
void print_freeRam() {
  Serial.println(freeRam());
}
#endif

/* ----------------------------------------------------------------------------- */

void setup() { 

  SoftSerial.begin(9600);
  
  Serial.begin(57600);
  Serial.print("Booting ");
  for(int i=0; i<5; i++) { delay(1000); Serial.print("."); }
  Serial.println();
  
  checkNetwork();

  #ifdef DEBUG
  timer.every(1000,  print_freeRam);
  #endif
  
  timer.every(10000, checkNetwork);

  fb.begin();
  
  // Define o pino como INPUT_PULLUP utilizando o resistor interno do Arduino
  
  fb.pinMode(INPUT_PIN, INPUT_PULLUP);
  
}

/* ----------------------------------------------------------------------------- */

void redirect() {

  while (SoftSerial.available()) Serial.write((char) SoftSerial.read()); 

  while (Serial.available()) {
    char c = Serial.read();
    if(c == '\n') {
      SIM900.println(buffer.get().c_str());
      buffer.clear();
    } 
    else buffer.addChar(c);
  } 

}

/* ----------------------------------------------------------------------------- */

void check_input_pin() {
  
  fb.update();

  if(fb.digitalChanged(INPUT_PIN)) {
    
    if (fb.digitalIsLow(INPUT_PIN)) {
     
       if(hasNetwork) {
           IF_DEBUG(Serial.println("Enviando SMS ..."))
           SIM900.sendSMS(NUMBER_SMS, MESSAGE_SMS);
       } else {
           IF_DEBUG(Serial.println("SEM REDE para enviar SMS ..."))
       }
    }
  }
  
}

/* ----------------------------------------------------------------------------- */

void loop() {

  timer.update();  
  check_input_pin();
  #ifdef DEBUG
  redirect();
  #endif
  
}

/* ------------------------------------------------------------------------ */

#ifdef DEBUG
int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#endif
