#ifndef AlarmSensor_H
#define AlarmSensor_H

#include <Arduino.h>
#include "../DigitalSensor/DigitalSensor.h"

class AlarmSensor : public DigitalSensor
{
public:
   AlarmSensor(byte pin) : DigitalSensor(pin, INPUT_PULLUP){}
};

#endif
