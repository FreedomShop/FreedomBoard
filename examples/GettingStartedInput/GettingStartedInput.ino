/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: GettingStartedInput
 
   Este exemplo mostra a facilidade de uso dos pinos de entrada da FreedomBoard e sua
   interacao com os reles. Neste exemplo, sempre que o pino INPUT_PIN for acionado, o
   rele OUTPUT_RELAY sera ligado por MILLIS_ON millisegundos
   
   Os pinos de entrada da FreedomBoard sao numerados de 0 a 10. Digitais (0-6) e Analogicos (7-10)
   
*/

#include <freedomboard.h>

#define DEBUG                       // Mostra as mensagens
#ifdef  DEBUG
  #define IF_DEBUG(x) x;
#else
  #define IF_DEBUG(x)
#endif

#define INPUT_PIN    2              // Pino que se deseja verificar o input
#define OUTPUT_RELAY 2              // Rele que se deseja acionar se o pino for para LOW (acionado)
#define MILLIS_ON    5000           // Tempo em millis que o rele deve ficar armado

FreedomBoard   fb;                  // Freedom Board

/* ----------------------------------------------------------------------------- */

void setup() { 

  Serial.begin(57600);
  Serial.println("examples\\GettingStartedInput - Verificando um pino de input");
  
  fb.begin();
  
  // Define o pino como INPUT_PULLUP utilizando o resistor interno do Arduino
  
  fb.pinMode(INPUT_PIN, INPUT_PULLUP);
  
}

/* ----------------------------------------------------------------------------- */

void loop() {
  
  fb.update();

  if(fb.digitalChanged(INPUT_PIN)) {
    
    IF_DEBUG(Serial.print(INPUT_PIN))
    
    if (fb.digitalIsLow(INPUT_PIN)) {
            
       fb.timerRelay(OUTPUT_RELAY, ON, MILLIS_ON, OFF);
       IF_DEBUG(Serial.println(" ficou LOW. Ligando o rele com timer"))
       
    } else {       
      
       IF_DEBUG(Serial.println(" ficou HIGH"))
       
    }  

  }
  
}


