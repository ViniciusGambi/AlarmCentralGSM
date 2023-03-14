#include "Arduino.h"
#include "DigitalEmitter.h"

DigitalEmitter::DigitalEmitter(byte pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

DigitalEmitter::DigitalEmitter(byte pin, boolean invert)
{
    _pin = pin;
    _invert = invert;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
}

void DigitalEmitter::turnOn()
{
    if (_invert) {
        return digitalWrite(_pin, LOW); 
    } else {
        return digitalWrite(_pin, HIGH); 
    }
}

void DigitalEmitter::turnOff()
{
    if (_invert) {
        return digitalWrite(_pin, HIGH); 
    } else {
        return digitalWrite(_pin, LOW); 
    }
}

