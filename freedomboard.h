#include <Arduino.h>
#include <stdint.h>

#ifndef FREEDOMBOARD_H
#define FREEDOMBOARD_H

#define NUM_RELAYS   4
#define NUM_DIGITAL  7
#define NUM_ANALOGIC 3
#define NUM_PINS     11

#define RELAY1_PIN 4
#define RELAY2_PIN 3
#define RELAY3_PIN 5
#define RELAY4_PIN 6

#define IOD_PIN1 7
#define IOD_PIN2 8
#define IOD_PIN3 9
#define IOD_PIN4 10
#define IOD_PIN5 11
#define IOD_PIN6 12
#define IOD_PIN7 13

#define IOA_PIN1 A0
#define IOA_PIN2 A1
#define IOA_PIN3 A2

#define INPUT_ZC 2

#define ON 1
#define OFF 0
#define INVERT 2
#define PREVIOUS 3

#define DIGITAL  1
#define ANALOGIC 2

static uint8_t RELAY_PINS[4]  = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN};
static uint8_t IO_PINS[10]    = {IOD_PIN1, IOD_PIN2, IOD_PIN3, IOD_PIN4, IOD_PIN5, IOD_PIN6, IOD_PIN7, IOA_PIN1, IOA_PIN2, IOA_PIN3};

struct PinsStatus {
  
  uint8_t pin;
  uint8_t type;
  uint8_t pinMode;
  uint8_t lastButtonState;
  long    lastDebounceTime;
  uint8_t buttonState;
  uint8_t changed;

};

struct RelayStatus {
  uint8_t pin;
  uint8_t relayState;
  uint8_t previousRelayState;
  uint32_t startOfTimer;
  uint32_t millisOfTimer; 
};

class FreedomBoard {
  
private:

     RelayStatus   relayOutputs[NUM_RELAYS];
     PinsStatus    ioStatus[NUM_PINS];

     uint8_t analogic_state[NUM_ANALOGIC];
     uint8_t analogic_changed[NUM_ANALOGIC];
     
     uint8_t ac_detection;
     uint8_t ac_changed;
     uint8_t ac_state;
     void checkDigitalInput(uint8_t pin);
     
public:
     void begin();
     
     uint8_t setRelay(uint8_t num, uint8_t op);
     uint8_t timerRelay(uint8_t num, uint8_t op, uint32_t timer, uint8_t then);
     uint8_t getRelay(uint8_t num);

     uint8_t pinMode(uint8_t num, uint8_t mode, uint8_t type=0);
     uint8_t digitalRead(uint8_t num);
     void    digitalWrite(uint8_t num, uint8_t val);
     void    digitalPwm(uint8_t num, uint8_t val);
     
     void    acDetection(uint8_t enabled);
     bool    acDetectionEnabled();
     bool    acChanged();
     bool    acIsOn();
     bool    acIsOff();
     bool    acStatus();
     
     uint8_t analogRead(uint8_t num);
     void    analogWrite(uint8_t num, uint16_t val);

     uint8_t digitalChanged(uint8_t num);
     uint8_t digitalChangedAny();
     
     uint8_t digitalIsHigh(uint8_t num);
     uint8_t digitalIsLow(uint8_t num);
     
     uint8_t setDigitalPinMode(uint8_t pin, uint8_t mode);
     
     uint8_t setAnalogicPinMode(uint8_t pin, uint8_t mode);
     
     void update();
};

#endif
