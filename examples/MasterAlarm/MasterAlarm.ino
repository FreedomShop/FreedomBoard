#include <SoftwareSerial.h>
#include <StringParser.h>
#include <Timer.h>

#define   DEBUG

#define PHONEBOOK_ADM_POS     250   // Posicao da agenda aonde esta a senha do administrador
#define PHONEBOOK_USR_BEGIN   1     // Posicao da agenda aonde inicia os numeros que podem enviar comandos
#define PHONEBOOK_USR_END     10    // Posicao da agenda aonde termina os numeros que podem enviar comandos

#define MINUTES_FAIL          5     // Minutos sem REDE necessario para resetar o SIM900

#include "freedomboard.h"
#include "freedomsim900.h"

Timer          timer;               // Checar tarefas periodicamente
StringParser   buffer(64);          // Parser para extrair palavras
String         number;              // Ultimo numero que recebeu SMS

SoftwareSerial SoftSerial(8,7);     // Comunicacao entre FreedomBoard e SIM900

FreedomBoard   fb;                  // Freedom Board
FreedomSim900  SIM900(SoftSerial);  // Modulo SIM900

bool           hasNetwork = false;    // retorno de AT+CPAS (0 - hasNetwork)
char           password[] = "123456"; // Senha do administrador (Posicao 250 da agenda) - Default 123456
uint8_t        countFail  = 0;


// Variaveis no PROGMEM para economizar espaco

// Comandos que exigem SENHA
// Colocar um \0 a mais no final para identificar que e' o ultimo comando

static const char cmds_password[]  PROGMEM = "ADD\0DEL\0PWD\0\0"; 

// Comandos que exigem que o destino esteja cadastrado na agenda (Posicao 1 a 10)
// Colocar um \0 a mais no final para identificar que e' o ultimo comando

static const char cmds_phonebook[] PROGMEM = "L1\0L2\0L3\0L4\0D1\0D2\0D3\0D4\0I1\0I2\0I3\0I4\0\0"; 

#define CMD_ADD 0
#define CMD_DEL 1
#define CMD_PWD 2

#define CMD_L1 0
#define CMD_L2 1
#define CMD_L3 2
#define CMD_L4 3
#define CMD_D1 4
#define CMD_D2 5
#define CMD_D3 6
#define CMD_D4 7
#define CMD_I1 8
#define CMD_I2 9
#define CMD_I3 10
#define CMD_I4 11

/* ----------------------------------------------------------------------------- */

void checkNetwork() {
 
   // AT+CPAS (Deve retornar 0)
   int i = SIM900.activityStatus();
   #ifdef DEBUG
   Serial.print("activityStatus: "); Serial.println(i);
   #endif
   
   bool nowNetwork = (i == 0);
   
   // Se mudou o status de OFF para ON, recarrega a senha de administrador
   // que esta na AGENDA na posicao 250
   
   if((!hasNetwork) && (nowNetwork)) {
     String number, name;
     if(SIM900.readPhoneBook(PHONEBOOK_ADM_POS, number, name)) {
       if((number.length()) && (number.length() <= 6)) {
         strcpy(password, number.c_str());
         
         #ifdef DEBUG
         Serial.print("admPassword: "); Serial.println(password);
         #endif
       }
     }
   }
   
   hasNetwork = nowNetwork;
   
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
    if(number.length() >= 8) parse_buffer();
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

void parse_buffer() {
  
  StringParser command(64);
  String       cmd;
   
  uint8_t max_commands = 32;
  while(max_commands-- && buffer.length()) {

     cmd = buffer.nextWord(';');
     if(!cmd.length()) continue;
     
     command.setString(cmd);
     parse_command(number, command);  
  }
 
  buffer.clear();
  
}

/* ----------------------------------------------------------------------------- */

void parse_command(String& number, StringParser &command) {

  String cmd; cmd.reserve(16);
  String pwd; cmd.reserve(16);
  int    i;
  
  // Comandos devem sem: COMMAND PARAM1, PARAM2, PARAM3, ..., PARAMN
  
  command.setMaxWordLength(16);
  cmd = command.nextWord();
  cmd.toUpperCase();

  i = find_command(cmd, cmds_password);
  
  #ifdef DEBUG
  Serial.print("find_command: "); Serial.print(cmd); Serial.print(" "); Serial.println(i);
  #endif
  
  if(i >= 0) {
    
    // Esses comandos sao no formato
    // comando, senha_adm, param1, param2, ..., paramN
    
    // Como o primeiro parametro e' a senha, vai logo testar
    
    pwd = command.nextWord(',');

    String OK;
    if(pwd == password) 
       OK = parse_command_password(i, command) ? "OK" : "ERROR";
    else
       OK = "WRONG PASS";
       
    #ifdef DEBUG
    Serial.print("parse_command: "); Serial.println(OK);
    #endif
     
    SIM900.sendSMS(number.c_str(), OK.c_str());
    
    return;

  } 
  
  
  // ---------------------------------------------------------------------------
  
  i = find_command(cmd, cmds_phonebook);
  
  if (i>=0) {
   
    String name;
    int j = SIM900.findPhoneBookByNumber(number.c_str(), name, PHONEBOOK_USR_BEGIN, PHONEBOOK_USR_END); 
    
    if(j) {
      parse_command_phonebook(i, command);
    }
    
  }

}

/* ----------------------------------------------------------------------------- */

bool parse_command_password(int i, StringParser& buffer) {

  // Adiciona um novo telefone que pode enviar comandos
  // Parametro: pos    - Posicao para inserir
  //            number - Numero a inserir
  
  if(CMD_ADD == i) {
    
    int pos       = buffer.nextWord(',').toInt();
    String number = buffer.nextWord();
    if((pos >= PHONEBOOK_USR_BEGIN) && (pos <= PHONEBOOK_USR_END)) {
       return SIM900.addPhoneBook(pos, number.c_str(), "");
    } else return false;
    
    
  // Deleta um telefone que foi adicionado posteriormente
  // Parametro: pos - Posicao a deletar (0 - 10)
  
  } else if (CMD_DEL == i) {
    
    int pos = buffer.nextWord(',').toInt();    
    if((pos >= PHONEBOOK_USR_BEGIN) && (pos <= PHONEBOOK_USR_END)) {
       return SIM900.deletePhoneBook(pos);
    }
    
  // Muda a senha de administrador
  // Parametro: new_pwd - Nova Senha
  
  } else if (CMD_PWD == i) {    
    
    String new_pwd = buffer.nextWord();
    if((new_pwd.length()) && (new_pwd.length() <= 6)) {
      strcpy(password, new_pwd.c_str());
      return SIM900.addPhoneBook(PHONEBOOK_ADM_POS, new_pwd.c_str(), "senha");
    }
    else                      
      return false;
    
  }
}

/* ----------------------------------------------------------------------------- */

bool parse_command_phonebook(int i, StringParser& buffer) {
  
  int op, relay;
  
  // L0,L1,L2,L3 - Liga os reles
  // D0,D1,D2,D3 - Desliga os reles
  // I0,I1,I2,I3 - Inverte os reles
  // Parametros: timer - Timer em milissegundos 
  //             then  - Ap's 
  
  if(i >= CMD_L1 && i <= CMD_I3) {
    
    if      (i >= CMD_L1 && i <= CMD_L4) { op = ON;     relay=i-CMD_L1; }
    else if (i >= CMD_D1 && i <= CMD_D4) { op = OFF;    relay=i-CMD_D1; }
    else if (i >= CMD_I1 && i <= CMD_I4) { op = INVERT; relay=i-CMD_I1; }
    
    int timer = buffer.nextWord(',').toInt();
    int then  = buffer.nextWord(',').toInt();
    
    if(timer) fb.timerRelay(relay, op, timer, then);
    else      fb.setRelay  (relay, op);
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
