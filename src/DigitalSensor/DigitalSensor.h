#ifndef DigitalSensor_H
#define DigitalSensor_H

#include <Arduino.h>

class DigitalSensor
{
public:
   DigitalSensor(byte pin, byte mode);
   DigitalSensor(byte pin, byte mode, boolean invert);
   boolean read();
   
private:
   byte _pin;
   boolean _invert = false;
};


#endif
