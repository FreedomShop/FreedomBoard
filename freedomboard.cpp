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

     uint8_t pin                 = IO_PINS[i];
     ioStatus[i].pin             = pin;

  }
  
  ac_detection = 0;
  ac_changed = false;
  ac_state   = 1;
  
  delay(100);
  zero_cross();
  
};

// ----------------------------------------------------------------------------

void FreedomBoard::checkDigitalInput(uint8_t idx)
{
  PinsStatus    &button = ioStatus[idx];
  uint8_t       pin     = button.pin;
  uint8_t       debounceDelay = 10;
  

  button.changed = false;
  if((button.type != DIGITAL) || (button.pinMode == OUTPUT)) return;  

  // If the switch changed, due to noise or pressing:

  uint8_t reading = ::digitalRead(pin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  if (reading != button.lastButtonState) {
    // reset the debouncing timer
    button.lastDebounceTime = millis();
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
  
  for(int i=0; i<NUM_PINS; i++) {
     
      checkDigitalInput(i);
  }

  for(int i=0; i<NUM_RELAYS; i++) {
   
     RelayStatus& relay = relayOutputs[i];
     if(relay.millisOfTimer) {
       if(millis() - relay.startOfTimer > relay.millisOfTimer) {
         relay.millisOfTimer = 0;
         setRelay(i, relay.previousRelayState);
       }  
     } 
  }
  
  ac_changed = false;
  
  if(ac_detection) {
    
    uint8_t temp_state;

    if((millis() - last_zero_cross) > 1000) {
      //if(ac_state == 255) ac_state = 0;
      temp_state = 0;
    } else {
      //if(ac_state == 255) ac_state = 1;
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
  ::digitalWrite(relayOutputs[num].pin, new_op);
  
  return true;

}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::getRelay(uint8_t num) {

  uint8_t new_op;
  if(num>=NUM_RELAYS) return 0;
  
  return relayOutputs[num].relayState;
  
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::timerRelay(uint8_t num, uint8_t op, uint32_t timer, uint8_t then) {

  if(num>=NUM_RELAYS) return false;

  uint8_t previous_op = relayOutputs[num].relayState;
  setRelay(num, op);

  if(timer) {

     RelayStatus& relay = relayOutputs[num];
  
     if(then != PREVIOUS) previous_op = then;
  
     relay.millisOfTimer      = timer;
     relay.previousRelayState = previous_op;
     relay.startOfTimer       = millis();
  }  

  return true;
}

// ----------------------------------------------------------------------------

uint8_t FreedomBoard::pinMode(uint8_t num, uint8_t mode, uint8_t type) {

  if(num>=NUM_PINS) return false;  
  if(!type) type = num >= NUM_DIGITAL ? ANALOGIC : DIGITAL;
  uint8_t pin = ioStatus[num].pin;

  if((pin >= A0) && (mode == INPUT_PULLUP)) {
    ::pinMode(pin, INPUT);
    ::analogWrite(pin, HIGH); 
  } else {
    ::pinMode(pin, mode);
  }
  ioStatus[num].pinMode = mode;
  ioStatus[num].type    = type;
  if(mode != OUTPUT && type == DIGITAL) {
     uint8_t state                 = ::digitalRead(pin);
     ioStatus[num].buttonState     = state;
     ioStatus[num].lastButtonState = state;
  }
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
    ::pinMode(INPUT_ZC, INPUT_PULLUP);
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


