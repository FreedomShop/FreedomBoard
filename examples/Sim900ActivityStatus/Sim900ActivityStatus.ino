/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: Sim900ActivityStatus
 
   Este exemplo verifica a situacao do SIM900 a cada 10 segundos e se nao houver
   cobertura durante 5 minutos ele ira reinicializar o modulo. 
   Eh necessario baixar a biblioteca externa Timer para que o exemplo funcione
   corretamente.
   
*/


#include <SoftwareSerial.h>
#include <StringParser.h>
#include <Timer.h>

#include <freedomboard.h>
#include <freedomsim900.h>

#define DEBUG                       // Mostra as mensagens
#ifdef  DEBUG
  #define IF_DEBUG(x) x;
#else
  #define IF_DEBUG(x)
#endif

Timer          timer;               // Checar tarefas periodicamente
StringParser   buffer(64);          // Parser para extrair palavras
SoftwareSerial SoftSerial(8,7);     // Comunicacao entre FreedomBoard e SIM900

FreedomBoard   fb;                  // Freedom Board
FreedomSim900  SIM900(SoftSerial);  // Modulo SIM900

bool           hasNetwork  = false;  // retorno de AT+CPAS (0 - hasNetwork)
uint8_t        secondsFail = 0;

#define        MINUTES_FAIL  5    

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
     
     secondsFail += 10;
     if(secondsFail > MINUTES_FAIL * 60) { // 5 Minutes  
       SIM900.reset();
       secondsFail = 0;
     }
   }
}

/* ----------------------------------------------------------------------------- */

void setup() { 

  #ifdef DEBUG
  timer.every(1000,  print_freeRam);
  #endif
  
  timer.every(10000, checkNetwork);

  SoftSerial.begin(9600);
  fb.begin();

  Serial.begin(57600);
  Serial.println("examples\\Sim900ActivityStatus - Verificando a rede do SIM900");
  Serial.print("Booting ");
  for(int i=0; i<5; i++) { delay(1000); Serial.print("."); }
  Serial.println();
  
  checkNetwork();

}

/* ----------------------------------------------------------------------------- */
// Redireciona a saida do SIM900 para a entrada Serial e vice-versa
// Muito util para programacao do modulo quando em desenvolvimento

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


#ifdef DEBUG

/* ------------------------------------------------------------------------ */

void print_freeRam() {
  
  Serial.print("FreeRam: "); 
  Serial.print(freeRam());
  Serial.println(" bytes");
}
/* ------------------------------------------------------------------------ */

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#endif
