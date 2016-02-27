//AT+CMGF=1
//AT+CNMI=0,0

#include <SoftwareSerial.h>
#include <StringParser.h>

#define RESPONSE_TIMEOUT 0
#define RESPONSE_OK      1
#define RESPONSE_ERROR   2
#define RESPONSE_MATCH   3

#define AT     "AT"
#define QUOTE  "\""
#define CRLF   "\r\n"
#define COMMA  ","

#define SMS_UNREAD 0
#define SMS_READ   1
#define SMS_ALL    2

class FreedomSim900 {

  private:
  Stream& stream;
  uint8_t last_sms;
  uint8_t last_response;
  public:
  FreedomSim900(Stream& stream);
  size_t printn(int n, ...);
  void begin();
  void clear(uint16_t max_interchar=10);
  bool isOK();
  bool waitLine(StringParser& line, uint32_t max_millis=500, uint16_t max_interchar=100);
  bool waitForEndOfCommand(String& line);
  int  waitResponse(StringParser& line, char* command, uint32_t max_millis=500, uint16_t max_interchar=100 );
  void println(const char* line);
  void print(const char* line);
  void printQuote(const char* line);
  void printAT(const char* line);
  bool endOfCommand(String& line);
  
  
  
  int  activityStatus();
  int  findPhoneBookByNumber(const char* number, String& name, uint8_t begin=1, uint8_t end=250);
  int  findPhoneBookByName(const char* name, String& number);
  int  readPhoneBook(int num, String& number, String &name);
  int  deletePhoneBook(int num);
  int  addPhoneBook(int num, const char* number, const char* name);
  bool sendSMS(const char *number, const char* message);
  bool sendSMS(int phoneBook, const char* message);
  bool readSMS(int num, String& number, String& message, bool delete_after_read=false, int max_size=160);
  bool deleteSMS(int num);

  uint8_t nextSMS(uint8_t required_status=SMS_ALL);
  
  int  lastSMS() { return last_sms; };
  int  lastResponse() { return last_response; };
  
  void reset() { stream.println("AT+CFUN=1,1"); };
  
  
};


