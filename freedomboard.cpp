#include <Arduino.h>
#include "freedomboard.h"
#include <stdint.h>

uint32_t last_zero_cross = 0;

void zero_cross() {
  last_zero_cross = millis();
}
// ----------------------------------------------------------------------------

void FreedomBoard::begin() { 
  for(int i=0; i<NUM_RELAYS; i++) {
     RelayStatus& relay = relayOutputs[i];
     uint8_t pin = RELAY_PINS[i];

     relay.pin = pin;
     relay.relayState = 0;
     relay.millisOfTimer = 0;
     ::pinMode(pin, OUTPUT);
  }
  
  memset(ioStatus, 0, sizeof(ioStatus));
  
  for(int i=0; i<NUM_PINS; i++) {

     uint8_t pin = IO_PINS[i];

     pinMode(i, INPUT);
     ioStatus[i].pin = pin; 
     ioStatus[i].buttonState     = digitalRead(pin);
     ioStatus[i].lastButtonState = ioStatus[i].buttonState;

  }
  
  ac_detection = 0;
  ac_changed = 0;
  ac_state   = 255;
  
  delay(100);
  
};

void FreedomBoard::checkDigitalInput(uint8_t idx)
{
  PinsStatus    &button = ioStatus[idx];
  uint8_t       pin     = button.pin;
  uint8_t       debounceDelay = 10;
  

  button.changed = false;
  if((button.type != DIGITAL) || ((button.pinMode != INPUT) && button.pinMode != INPUT_PULLUP)) return;  

  // If the switch changed, due to noise or pressing:

  uint8_t reading   = digitalRead(pin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  if (reading != button.lastButtonState) {
    // reset the debouncing timer
    button.lastDebounceTime = millis();
    //Serial.println("reading != lastButtonState");
  }
  
  long m = millis();
  if ((m - button.lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != button.buttonState) {
      button.buttonState = reading;
      button.changed     = true;
    }
  } 

  button.lastButtonState = reading;
}

// ----------------------------------------------------------------------------

void FreedomBoard::update() { 
  
  for(int i=0; i<NUM_DIGITAL; i++) {
     
     if(ioStatus[i].pinMode == INPUT) {
        checkDigitalInput(i);
     }
  }

  for(int i=0; i<NUM_RELAYS; i++) {
   
     RelayStatus& relay = relayOutputs[i];
     if(relay.millisOfTimer) {
       if(millis() - relay.startOfTimer > relay.millisOfTimer) {
         relay.millisOfTimer = 0;
         setRelay(i+1, relay.previousRelayState);
       }  
     } 
  }
  
  ac_changed = false;
  
  if(ac_detection) {
    
    uint8_t temp_state;

    if(millis() - last_zero_cross > 1000) {
      if(ac_state == 255) ac_state = 0;
      temp_state = 0;
    } else {
      if(ac_state == 255) ac_state = 1;
      temp_state = 1;
    }
    
    ac_changed = (temp_state != ac_state);
    if(ac_changed) ac_state = temp_state;
    
  }
  
};

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::setRelay(uint8_t num, uint8_t op) {

  uint8_t new_op;
  if(num>=NUM_RELAYS) return false;
  
  if(op == INVERT) new_op = (1 - relayOutputs[num].relayState);
  else             new_op = op;
  
  relayOutputs[num].relayState = new_op;
  digitalWrite(relayOutputs[num].pin, new_op);
  
  return true;
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::timerRelay(uint8_t num, uint8_t op, uint32_t timer, uint8_t then) {

  uint8_t previous_op;
  if(num>=NUM_RELAYS) return false;

  RelayStatus& relay = relayOutputs[num];
  
  if(then == PREVIOUS) previous_op = relay.relayState;
  else                 previous_op = then;
  
  setRelay(num, op);
  
  relay.millisOfTimer      = timer;
  relay.previousRelayState = previous_op;
  relay.startOfTimer       = millis();
  
  return true;
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::pinMode(uint8_t num, uint8_t mode, uint8_t type) {

  if(num>=NUM_DIGITAL) return false;  
  if(!type) type = num >= NUM_DIGITAL ? ANALOGIC : DIGITAL;
  ::pinMode(ioStatus[num].pin, mode);
  ioStatus[num].pinMode = mode;
  ioStatus[num].type    = type;
  return true;
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::digitalRead(uint8_t num) {

  return (num<NUM_PINS) ? ::digitalRead(ioStatus[num].pin) : 0;
  
}


// ----------------------------------------------------------------------------

void FreedomBoard::digitalWrite(uint8_t num, uint8_t val) {

  if(num<NUM_PINS) ::digitalWrite(ioStatus[num].pin, val);
}

// ----------------------------------------------------------------------------

void FreedomBoard::digitalPwm(uint8_t num, uint8_t val) {

  if(num<NUM_PINS) ::analogWrite(ioStatus[num].pin, val);
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::analogRead(uint8_t num) {
  
  return (num<NUM_PINS) ? ::analogRead(ioStatus[num].pin) : 0;
}

// ----------------------------------------------------------------------------

void FreedomBoard::analogWrite(uint8_t num, uint16_t val) {

  if(num<NUM_PINS) ::analogWrite(ioStatus[num].pin, val);
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::digitalChanged(uint8_t num) {
 
  if(num>=NUM_DIGITAL) return false;
  return ioStatus[num].changed;
  
}  

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::digitalChangedAny() {

  for(int i=0; i<NUM_DIGITAL; i++) {
    if(ioStatus[i].changed) return true;
  }
  return false;
  
}  

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::digitalIsHigh(uint8_t num) {
 
  if(num>=NUM_DIGITAL) return false;
  return ioStatus[num].buttonState;
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::digitalIsLow(uint8_t num) {
 
  if(num>=NUM_DIGITAL) return false;
  return !ioStatus[num].buttonState;
}

// ----------------------------------------------------------------------------

void FreedomBoard::acDetection(uint8_t enabled) {
  ac_detection = enabled;
  
  if(enabled) {
    attachInterrupt(0, zero_cross, CHANGE);  
  } else {
    detachInterrupt(0);
  }
  
}

// ----------------------------------------------------------------------------

bool FreedomBoard::acDetectionEnabled() {

   return ac_detection;  
}

// ----------------------------------------------------------------------------

bool FreedomBoard::acChanged() {

   return ac_changed;  
}

// ----------------------------------------------------------------------------

bool FreedomBoard::acIsOn() {

   return ac_state;  
}

// ----------------------------------------------------------------------------

bool FreedomBoard::acIsOff() {

   return !ac_state;  
}

// ----------------------------------------------------------------------------

bool FreedomBoard::acStatus() {

   return ac_state;  
}


