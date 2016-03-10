/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: Sim900PhoneBook
 
   Este exemplo ira receber comandos via SMS para gerenciar a agenda do SIM900
   
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

#define PHONEBOOK_USR_BEGIN   1     // Posicao inicial da agenda para gerenciamento
#define PHONEBOOK_USR_END     10    // Posicao final da agenda para gerenciamento

#define MINUTES_FAIL          5     // Minutos sem REDE necessario para resetar o SIM900

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


// Variaveis no PROGMEM para economizar espaco

// Comandos para gerenciar a agenda
// Colocar um \0 a mais no final para identificar que e' o ultimo comando

static const char cmds_validate[]  PROGMEM = "ADD\0DEL\0\0"; 

#define CMD_ADD 0
#define CMD_DEL 1

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

void checkSMS() {

  if(!hasNetwork) return;
  
  String message; message.reserve(64);

  if(SIM900.readSMS(0, number, message, true, 64)) {
    
    #ifdef DEBUG
    Serial.print("checkSMS: ");
    Serial.print(number); 
    Serial.print(" - "); 
    Serial.println(message);
    #endif
    
    buffer.clear();
    buffer.addChars((char*) message.c_str());
    if(number.length() >= 8) parse_command(buffer);
  }

}

/* ----------------------------------------------------------------------------- */

#ifdef DEBUG
void print_freeRam() {
  Serial.println(freeRam());
}
#endif


/* ----------------------------------------------------------------------------- */

int find_command(String& cmd, const char * pgm_cmds) {

  char  bufstr[16];
  char* cmdstr;
  int   size = 0;
  int   i    = 0;
  while(true) {
    cmdstr = (char*)(pgm_cmds + size);
    strcpy_P(bufstr, cmdstr);
    if(!*bufstr) break;
    if(cmd == bufstr) return i;
    size+=strlen(bufstr)+1;
    i++;
  }
  return -1;  
}

/* ----------------------------------------------------------------------------- */

bool parse_command(StringParser &command) {

  String cmd; cmd.reserve(16);
  String pwd; cmd.reserve(16);
  int    i;
  
  // Comandos devem sem: COMMAND PARAM1, PARAM2, PARAM3, ..., PARAMN
  
  command.setMaxWordLength(16);
  cmd = command.nextWord();
  cmd.toUpperCase();

  i = find_command(cmd, cmds_validate);
  
  #ifdef DEBUG
  Serial.print("find_command: "); Serial.print(cmd); Serial.print(" "); Serial.println(i);
  #endif
    
  // Adiciona um telefone novo na agenda
  // Parametro: pos - Posicao a Adicionar (1 - 10)
  //            numero - Numero a adicionar
  
  if(CMD_ADD == i) {
    
    int pos       = buffer.nextWord(',').toInt();
    String number = buffer.nextWord();
    if((pos >= PHONEBOOK_USR_BEGIN) && (pos <= PHONEBOOK_USR_END)) {
       return SIM900.addPhoneBook(pos, number.c_str(), "");
    } else return false;
    
    
  // Deleta um telefone que foi adicionado posteriormente
  // Parametro: pos - Posicao a deletar (1 - 10)
  
  } else if (CMD_DEL == i) {
    
    int pos = buffer.nextWord(',').toInt();    
    if((pos >= PHONEBOOK_USR_BEGIN) && (pos <= PHONEBOOK_USR_END)) {
       return SIM900.deletePhoneBook(pos);
    }
      
  } 
  
}


/* ----------------------------------------------------------------------------- */

void setup() { 

  #ifdef DEBUG
  timer.every(1000,  print_freeRam);
  #endif
  
  timer.every(5000,  checkSMS);
  timer.every(10000, checkNetwork);

  SoftSerial.begin(9600);
  
  number.reserve(16); 

  Serial.begin(57600);
  Serial.print("Booting ");
  for(int i=0; i<5; i++) { delay(1000); Serial.print("."); }
  Serial.println();
  
  checkNetwork();

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
