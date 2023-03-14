#ifndef Relay_H
#define Relay_H

#include <Arduino.h>
#include "../DigitalEmitter/DigitalEmitter.h"

class Relay : public DigitalEmitter
{
public:
   Relay(byte pin) : DigitalEmitter(pin){}
   Relay(byte pin, boolean invert) : DigitalEmitter(pin, invert){}
};

#endif
