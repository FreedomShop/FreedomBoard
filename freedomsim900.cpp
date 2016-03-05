//Time: AT+CCLK?

#include <Arduino.h>
#include <StringParser.h>
#include <freedomsim900.h>

// ----------------------------------------------------------------------------

FreedomSim900::FreedomSim900(Stream& serial) : stream(serial) {
 
}

// ----------------------------------------------------------------------------

void FreedomSim900::begin() 
{
}

// ----------------------------------------------------------------------------

void FreedomSim900::clear(uint16_t max_interchar) 
{
   uint32_t last_char = 0;
  
   while(true) {   
     
     uint32_t now = millis();
     if (!last_char) last_char = now;
     else if((now - last_char) > max_interchar) break;
         
     if(stream.available()) {
       last_char = now;
       stream.read();
     }
  }
  
}
// ----------------------------------------------------------------------------
/*
size_t redirect_printf(Stream& stream, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char temp[128];
  size_t len = vsnprintf(temp, 128, format, arg);
  va_end(arg);
  stream.print(temp);
  return len;
}
*/
// ----------------------------------------------------------------------------
/*
size_t FreedomSim900::printn(int n, ...) {
  va_list     arg;
  const char* param;
  
  clear();
  
  va_start(arg, n);
  for (int i=0;i<n;i++)
  {
    param=va_arg(arg, const char*);
    stream.print(param);
  }
  va_end(arg);
}
*/
// ----------------------------------------------------------------------------------------------------

bool FreedomSim900::isOK() {
  
   //static uint32_t now         = 0;
   bool     last_result = false;
   
   //if((now) && (millis() - now) < 10000) return last_result;
   
   StringParser buffer(10);
   
   println(AT);
   //now = millis();
   while(true) {
     
       if(!(last_result = waitLine(buffer)))   break;       
       if(last_result   = endOfCommand(buffer.get())) break;
   }
   
   return last_result;
}

// ----------------------------------------------------------------------------

bool FreedomSim900::endOfCommand(String& line) {
  return ((line == "OK") || (line == "ERROR"));
}

// ----------------------------------------------------------------------------

bool FreedomSim900::waitForEndOfCommand(String& line) {
  
  StringParser buffer(16);
  while(true) {
     if(!waitLine(buffer)) return false;
     if(endOfCommand(buffer.get())) break;
  }
  return true;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::waitResponse(StringParser& line, char* command, uint32_t max_millis, uint16_t max_interchar ) {

  while(true) {
    
     if(!waitLine(line, max_millis, max_interchar)) { 
        last_response = RESPONSE_TIMEOUT; 
        break; 
     }

     String aux = line.nextWord(':');
     
     if (aux == command)    { 
        last_response = RESPONSE_MATCH; 
        break; 
     }
     
     if (endOfCommand(aux)) { 
        last_response = (aux == "OK" ? RESPONSE_OK : RESPONSE_ERROR); 
        break; 
     }
     
  }
  
  return last_response;
}

// ----------------------------------------------------------------------------

int FreedomSim900::activityStatus() {
  
  #define CPAS   "AT+CPAS"
  #define CPAS_R "+CPAS"
  
  int          result = 9;
  StringParser buffer(64);

  println(CPAS);
  
  if((waitResponse(buffer, CPAS_R)) == RESPONSE_MATCH) {
     result = buffer.nextWord().toInt();
     //clear();
  }

  return result;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::findPhoneBookByName(const char* name, String& number) {
  
  #define CPBF   "AT+CPBF="
  #define CPBF_R "+CPBF"
  
  int          result = 0;
  String       aux;
  StringParser buffer(64);
  
  aux.reserve(16);
  //printn(5, CPBF, QUOTE, name, QUOTE, CRLF);
  print(CPBF); printQuote(name); print(CRLF);

  if(waitResponse(buffer, CPBF_R) == RESPONSE_MATCH) {
     result = buffer.nextWord(',').toInt();
     number = buffer.nextWord(',');
     //clear();
  }
  
  return result;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::readPhoneBook(int num, String& number, String &name) {
  
  #define CPBR   "AT+CPBR="
  #define CPBR_R "+CPBR"
  
  int          result = false;
  char         str_num[6];   itoa(num, str_num, 10);
  StringParser buffer(64);
  
  // printn(3, CPBR, str_num, CRLF);
  print(CPBR); println(str_num); //print(CRLF);
  
  int r;
    
  if(waitResponse(buffer, CPBR_R) == RESPONSE_MATCH) {

     buffer.nextWord(',');
     number = buffer.nextWord(',');
     buffer.nextWord(',');
     name   = buffer.nextWord(',');
     result = 1;
     
     //clear();
  }
  
  return result;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::deletePhoneBook(int num) {
  
  #define CPBW   "AT+CPBW="
  
  int  result = 0;
  char str_num[6];          
  StringParser buffer(64);
  
  itoa(num, str_num, 10);
  print(CPBW); println(str_num); // print(CRLF);
  //printn(2, CPBW, str_num);

  if((waitLine(buffer)) &&
     (endOfCommand(buffer.get()))) result = 1;
    
  return result;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::addPhoneBook(int num, const char* number, const char* name) {
  
  #define CPBW   "AT+CPBW="
  
  int  result = 0;
  char str_num[6];          
  StringParser buffer(64);
  
  itoa(num, str_num, 10);
  print(CPBW); 
  if(num) print(str_num); 
  print(COMMA); printQuote(number); print(COMMA); print(COMMA); printQuote(name); print(CRLF);
  
  if((waitLine(buffer)) &&
     (endOfCommand(buffer.get()))) result = 1;
    
  return result;
  
}

// ----------------------------------------------------------------------------

int FreedomSim900::findPhoneBookByNumber(const char* number, String& name, uint8_t begin, uint8_t end) {
  
  #define CPBR   "AT+CPBR="
  #define CPBR_R "+CPBR"
  
  int          pos, result = 0;
  bool         finished = false;
  String       str_number(number);
  StringParser buffer(64);
  char         str_i[6], str_j[6];
  while(begin < end) {

    uint8_t i = begin;
    uint8_t j = min(end, i+4);
    
    itoa(i, str_i, 10); itoa(j, str_j, 10);
    print(CPBR); print(str_i); print(COMMA); println(str_j); //print(CRLF);
    //printn(5, CPBR, str_i, COMMA, str_j, CRLF); 
  
    while(waitResponse(buffer, CPBR_R) == RESPONSE_MATCH) {
       
       pos = buffer.nextWord(',').toInt();
       String aux = buffer.nextWord(',');
       if((!result) && ((aux.endsWith(number)) || str_number.endsWith(aux))) {
         result = pos;
         buffer.nextWord(',');
         name = buffer.nextWord(',');
       }

    }
    
    //clear();
    
    begin = j+1;
    
    if(result) break;
    
  }
  
  return result;
  
}

// ----------------------------------------------------------------------------

bool FreedomSim900::waitLine(StringParser& line, uint32_t max_millis, uint16_t max_interchar) {
  
  uint32_t now       = millis();
  uint32_t last_char = 0;
  
  line.clear();
  
  bool forever = !max_millis;
  bool timeout = false;
  
  do {

    if(stream.available()) {
      
      now    = millis();
      
      if (!last_char) last_char = now;
      else if((now - last_char) > max_interchar) {
        
        clear();
        // while(stream.available()) stream.read();
        timeout = true;
        break; 
        
      }
      
      char c    = stream.read();
      last_char = now;
      
      if(c == '\n') {
        if (line.length()) break;
      } else if(c != '\r') line.addChar(c);
    }    
    
    if(timeout = (!(forever) && (millis() - now > max_millis))) break;

  } while(1);

  line.get().trim();
  return !timeout;  
}

// ----------------------------------------------------------------------------

bool FreedomSim900::sendSMS(int phoneBook, const char* message) {
 
   String number, name;
   readPhoneBook(phoneBook, number, name);
   if(number.length()) return sendSMS(number.c_str(), message); 
   else return false;
}

// ----------------------------------------------------------------------------

bool FreedomSim900::sendSMS(const char *number, const char* message) {
  
  #define CMGS   "AT+CMGS="
  #define CMGS_R "+CMGS"

  bool         result = false;
  bool         send = false;
  
  StringParser buffer(64);

  if(strlen(message) > 159) return 0;
  
  print(CMGS); printQuote(number); print(CRLF);
  //printn(5, CMGS, QUOTE, number, QUOTE, CRLF);
  
  waitLine(buffer);
  if(buffer.get() == ">") {
    
    stream.print(message);
    stream.print((char) 26);
    
    if(waitResponse(buffer, CMGS_R, 60000) == RESPONSE_MATCH) {

       String aux = buffer.nextWord(':');
       if(aux != "ERROR") {
         last_sms = aux.toInt();
         result = true;
       }
       
       //clear();
    }
    
  }
  
  return result;
    
}

// ----------------------------------------------------------------------------

bool FreedomSim900::readSMS(int num, String& number, String& message, bool delete_after_read, int max_size) {
  
  #define CMGR   "AT+CMGR="
  #define CMGR_R "+CMGR"

  bool         result = false;
  char         str_num[6];   

  StringParser buffer(160);
  message = "";
  
  if(!num) num = nextSMS(SMS_ALL);
  if(!num) return false;
  
  last_sms = num;
  itoa(num, str_num, 10);
  clear();
 
  print(CMGR); print(str_num); print(CRLF);
  //printn(5, CMGS, QUOTE, number, QUOTE, CRLF);
    
  while(true) {
    
    if(!waitLine(buffer)) break;
    if(endOfCommand(buffer.get())) break;
    
    if(buffer.get().startsWith(CMGR_R)) {
      
      buffer.nextWord(',');
      number = buffer.nextWord(',');
      
    } else {
      
      String& s = buffer.get();
      int len   = message.length(); 
      
      if(len) message += ' ';   
      
      if(len < max_size) {
        message += s.substring(0, max_size - len); 
      }      
      
    }
    
  }
  
  if(delete_after_read) deleteSMS(num);
  
  return message.length();
  
}

// ----------------------------------------------------------------------------

bool FreedomSim900::deleteSMS(int num) {
  
  #define CMGD   "AT+CMGD="
  
  bool  result = false;
  char str_num[6];          
  StringParser buffer(64);
  
  itoa(num, str_num, 10);
  print(CMGD); println(str_num); // print(CRLF);

  result = (waitLine(buffer)) &&
           (endOfCommand(buffer.get()));
    
  return result;
  
}

// ----------------------------------------------------------------------------

uint8_t FreedomSim900::nextSMS(uint8_t required_status) {
  
  #define CMGL_R "+CMGL"

  uint8_t      result = 0;
  bool         send = false;
  char         str_aux[32];
  
  StringParser buffer(64);
  
  switch (required_status) {
  case SMS_UNREAD:
       strcpy_P(str_aux, PSTR("AT+CMGL=\"REC UNREAD\""));
       break;
  case SMS_READ:
       strcpy_P(str_aux, PSTR("AT+CMGL=\"REC READ\""));
       break;
  case SMS_ALL:
       strcpy_P(str_aux, PSTR("AT+CMGL=\"ALL\""));
       break;
  }
  
  //clear();
  println(str_aux);
  if(waitResponse(buffer, CMGL_R) == RESPONSE_MATCH) {
    result = buffer.nextWord(',').toInt();
  }
  clear();
  
  return result;
  
}

// ----------------------------------------------------------------------------

void FreedomSim900::println(const char* line) {
  clear();
  stream.println(line);
}

// ----------------------------------------------------------------------------

void FreedomSim900::print(const char* line) {
  clear();
  stream.print(line);
}

// ----------------------------------------------------------------------------

void FreedomSim900::printQuote(const char* line) {
  clear();
  stream.print("\""); stream.print(line); stream.print("\"");
}

// ----------------------------------------------------------------------------

void FreedomSim900::printAT(const char* line) {
  clear();
  stream.print("AT"); stream.print(line); 
}

