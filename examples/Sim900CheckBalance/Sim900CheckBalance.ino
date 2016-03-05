/*

 Copyright (C) 2016 Wellington Rats <wellrats@gmail.com>, <wellrats@freedomshop.com.br>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Exemplo: Sim900CheckBalance
 
   Este exemplo verificar junto a operadora o saldo em R$ de creditos
   do SIMCARD inserido no SIM900.
   Ao receber um SMS com o comando SALDO, ele enviara uma solicitacao
   a operadora e o retorno sera enviado ao numero que enviou primeiramente
   o comando SALDO
   
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

#define MINUTES_FAIL          5      // Minutos sem REDE necessario para resetar o SIM900
#define PHONEBOOK_USR_BEGIN   1      // Posicao da agenda aonde inicia os numeros que podem enviar comandos
#define PHONEBOOK_USR_END     10     // Posicao da agenda aonde termina os numeros que podem enviar comandos
#define NUMBER_BALANCE        "#9999" // Numero ao qual sera enviada para checar o saldo
#define MESSAGE_BALANCE       "Mensagem para checar o saldo"       // Mensagem a ser enviada
#define MESSAGE_SENDED        "Saldo solicitado para a operadora"  // Mensagem de confirmacao do comando

#define WORD_BALANCE          "saldo" // Palavra que deve ser checada na mensagem para saber
                                      // se foi o retorno da solicitacao de saldo
                                      
#define CHECK_PHONEBOOK             // So ira enviar de volta o SMS se o numero recebido
                                    // Estiver na posicao de 1 a 10 da agenda
//#undef CHECK_PHONEBOOK            // Ira aceitar SMS de qualquer numero

#include <freedomsim900.h>

Timer          timer;               // Checar tarefas periodicamente
StringParser   buffer(64);          // Parser para extrair palavras
String         number;              // Ultimo numero que recebeu SMS
String         last_number;         // Ultimo numero que solicitou o SALDO

SoftwareSerial SoftSerial(8,7);     // Comunicacao entre FreedomBoard e SIM900

FreedomSim900  SIM900(SoftSerial);  // Modulo SIM900

bool           hasNetwork = false;  // retorno de AT+CPAS (0 - hasNetwork)
uint8_t        countFail  = 0;


// Variaveis no PROGMEM para economizar espaco

// Comandos que exigem SENHA
// Colocar um \0 a mais no final para identificar que e' o ultimo comando

static const char cmds_validate[]  PROGMEM = "SALDO\0\0"; 

#define CMD_SALDO 0

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
  
  String message; message.reserve(160);

  if(SIM900.readSMS(0, number, message, true)) {
    
    #ifdef DEBUG
    Serial.print("checkSMS: ");
    Serial.print(number); 
    Serial.print(" - "); 
    Serial.println(message);
    #endif
    
    buffer.clear();
    buffer.addChars((char*) message.c_str());
    if(number.length() >= 8) parse_command(buffer);
    else if(number == NUMBER_BALANCE && 
       message.indexOf(WORD_BALANCE) >= 0 &&
       last_number.length() > 0) {
    
       SIM900.sendSMS(last_number.c_str(), message.c_str());
       
      
    }

    // Para garantir que so vai enviar o saldo uma vez
    last_number = "";

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

  // Vai procurar o comando que chegou na lista de comandos 
  // possiveis que estao armazenadas no PROGMEM
  // Lembrando que o final da lista deve terminar com \0\0
  
  // Se achar o comando retorna sua posicao de 0 a n - 1
  // Se nao achar retorna -1
  
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

void parse_command(StringParser &command) {

  // Se a diretiva de compilacao abaixo esta ativa
  // so aceita comandos se o numero estiver cadastrado na agenda
  
  #ifdef CHECK_PHONEBOOK
  
  String name;
  bool OK = SIM900.findPhoneBookByNumber(number.c_str(), name, PHONEBOOK_USR_BEGIN, PHONEBOOK_USR_END); 
  if(!OK) return;
  
  #endif
  
  String cmd; cmd.reserve(16);
  String pwd; cmd.reserve(16);
  int    i;
  
  // Comandos devem sem: COMMAND PARAM1, PARAM2, PARAM3, ..., PARAMN
  // Limita o tamanho do comando para 16 posicoes
  
  command.setMaxWordLength(16);
  
  // Vai pegar a primeira palavra que veio no SMS
  cmd = command.nextWord();
  cmd.toUpperCase();
  
  i = find_command(cmd, cmds_validate);
  
  #ifdef DEBUG
  Serial.print("find_command: "); Serial.print(cmd); Serial.print(" "); Serial.println(i);
  #endif
  
  if(i == CMD_SALDO) {
    last_number = number;
    SIM900.sendSMS(NUMBER_BALANCE, MESSAGE_BALANCE);
    SIM900.sendSMS(last_number.c_str(), MESSAGE_SENDED);
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
  
  // Eh importante que toda String que seja definida no inicio do Sketch
  // tenham uma quantidade de bytes reservada a fim de evitar que o heap 
  // diminua de tamanho ao longo do tempo
  
  number.reserve(16); 
  last_number.reserve(16);

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
