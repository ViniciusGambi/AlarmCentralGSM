#ifndef DigitalEmitter_H
#define DigitalEmitter_H

#include <Arduino.h>

class DigitalEmitter
{
public:
   DigitalEmitter(byte pin);
   DigitalEmitter(byte pin, boolean invert);
   void turnOn();
   void turnOff();
   
private:
   byte _pin;
   boolean _invert = false;
};


#endif
