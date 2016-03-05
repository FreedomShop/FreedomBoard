/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: Sim900ReceiveSms
 
   Este exemplo mostra a facilidade para receber mensagens via SMS utilizando a biblioteca
   freedomsim900
   
*/

#include <SoftwareSerial.h>
#include <StringParser.h>
#include <Timer.h>

#define   DEBUG

#define MINUTES_FAIL          5     // Minutos sem REDE necessario para resetar o SIM900

#include <freedomboard.h>
#include <freedomsim900.h>

Timer          timer;               // Checar tarefas periodicamente
StringParser   buffer(64);          // Parser para extrair palavras
String         number;              // Ultimo numero que recebeu SMS

SoftwareSerial SoftSerial(8,7);     // Comunicacao entre FreedomBoard e SIM900

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
   
   bool hasNetwork = (i == 0);
   
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

void checkSMS() {

  if(!hasNetwork) return;
  
  String number;  number.reserve(16);
  String message; message.reserve(160);

  if(SIM900.readSMS(0, number, message, true)) {
    
    #ifdef DEBUG
    Serial.print("checkSMS: ");
    Serial.print(number); 
    Serial.print(" - "); 
    Serial.println(message);
    #endif
    
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
  
  timer.every(5000,  checkSMS);
  timer.every(10000, checkNetwork);

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

void loop() {

  timer.update();  
  
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
