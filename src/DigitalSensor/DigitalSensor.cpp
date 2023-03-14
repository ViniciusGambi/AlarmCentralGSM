#include "Arduino.h"
#include "DigitalSensor.h"

DigitalSensor::DigitalSensor(byte pin, byte mode)
{
    _pin = pin;
    pinMode(_pin, mode);
}

DigitalSensor::DigitalSensor(byte pin, byte mode, boolean invert)
{
    _pin = pin;
    _invert = invert;
    pinMode(_pin, mode);
}

boolean DigitalSensor::read()
{
    if (_invert){
        return !digitalRead(_pin); 
    } else {
        return digitalRead(_pin); 
    }
    
}
