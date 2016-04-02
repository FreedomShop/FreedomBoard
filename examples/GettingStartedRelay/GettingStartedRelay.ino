/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: GettingStartedRelay
 
   Este exemplo demonstra a facilidade para a utilizacao dos 4 reles disponiveis
   na FreedomBoard.
   A cada um segundo, um rele sera acionado na sequencia e depois de um segundo
   sera apagado
   
*/

#include <freedomboard.h>

#define DEBUG                       // Mostra as mensagens
#ifdef  DEBUG
  #define IF_DEBUG(x) x;
#else
  #define IF_DEBUG(x)
#endif

FreedomBoard   fb;                  // Freedom Board

/* ----------------------------------------------------------------------------- */

void setup() { 

  Serial.begin(57600);
  Serial.println();
  Serial.println("examples\\GettingStartedRelay - Checking FreedomBoard Status");
  fb.begin();
  
}

/* ----------------------------------------------------------------------------- */

int rele = 0;
int op   = ON;
long now = millis();

void loop() {
  
  fb.update();

  if(millis() - now > 1000) { 
    now = millis();
    
    IF_DEBUG(Serial.print(op ? "Ligando rele " : "Desligando rele "));
    IF_DEBUG(Serial.println(rele));
    
    // Aciona um rele, ligando ou desligando
    fb.setRelay(rele, op);
    
    // Inverte o valor da variavel op e verifica se o novo valor eh ON
    if(op = (1-op)) {
      // Vai para o proximo rele
      rele = (rele + 1) % NUM_RELAYS;
    }
  } 
}


