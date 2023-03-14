#include <SoftwareSerial.h>
#include "CONFIG.h"
#include "src/DigitalSensor/DigitalSensor.h"
#include "src/Relay/Relay.h"
#include "src/AlarmSensor/AlarmSensor.h"

#define TX_PIN 9
#define RX_PIN 10
#define BUTTON_PIN 3
#define S1_PIN 2
#define BIP_INTERVAL 500

DigitalSensor button = DigitalSensor(BUTTON_PIN, INPUT_PULLUP);
Relay siren = Relay(7, true); //inversao do rele (NF/NA)
SoftwareSerial serialGSM(TX_PIN, RX_PIN);

bool hasSMS = false;
String smsPhoneNumber;
String smsMessage;
String gsmCommand = "";
String lastGSM = "";
String stopCallingPasswordInput = "";

unsigned long alarmTime = 0;
unsigned long preTriggerTime = 0;
unsigned long triggerTime = 0;
unsigned long lastCallTime = 0;

boolean alarmed = false;
boolean preTriggered = false;
boolean triggered = false;
boolean callAccepted = false;

boolean buttonPressed = false;
boolean moved = false;

void buttonPressedInt();
void movedInt();
void beep(int interval, byte times);
void alarm();
void unAlarm();
void preTrigger();
void trigger();
void untrigger();
void setupGSM();
void readGSM();
void makeCall(String phoneNumber);
void rejectCall();

void setup()
{
  Serial.begin(9600);

  serialGSM.listen();
  serialGSM.begin(9600);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressedInt, FALLING);
  attachInterrupt(digitalPinToInterrupt(S1_PIN), movedInt, RISING);

  buttonPressed = false;
  alarmed = false;

  delay(5000);

  setupGSM();
}

void loop()
{
  readGSM();
  if (gsmCommand != "")
  {
    Serial.println(gsmCommand);
    lastGSM = gsmCommand;
    gsmCommand = "";
  }

  // Write Serial Messages to GSM module
  if (Serial.available())
  {
    serialGSM.write(Serial.read());
  }

  // Alarming, unalarming and untrigger
  if (buttonPressed)
  {
    if (triggered)
    {
      untrigger();
    }
    else
    {
      if (alarmed)
      {
        unAlarm();
      }
      else
      {
        alarm();
      }
    }
    buttonPressed = false;
  }

  // Pre-trigger
  if (moved)
  {
    if (alarmed && !triggered && !preTriggered && ((millis() - alarmTime) > (unsigned long)(SECONDS_TO_LEAVE * 1000)))
    {
      preTrigger();
    }
    moved = false;
  }

  // Trigger
  if (preTriggered && !triggered && (millis() - preTriggerTime) > (unsigned long)(SECONDS_TO_TRIGGER * 1000))
  {
    trigger();
  }

  // Untrigerring by time exceed
  if (triggered && ((millis() - triggerTime) > (unsigned long)TRIGGERED_MINUTES_LIMIT * 60 * 1000))
  {
    untrigger();
  }

  // Call management
  if (triggered && !callAccepted && (millis() - lastCallTime > (unsigned long)CALL_MINUTES_INTERVAL * 60 * 1000))
  {
    makeCall(PHONE_NUMBER);
    lastCallTime = millis();
  }

  // dtmf
  if (lastGSM.indexOf("+DTMF:") > -1)
  {
    stopCallingPasswordInput += lastGSM.charAt(9);
    if (stopCallingPasswordInput.indexOf(STOP_CALLING_PASSWORD) > -1){
      callAccepted = true;
      Serial.println("Stop calling");
    }
    lastGSM = "";
  }

  // SMS control
  if (hasSMS)
  {
    Serial.println(smsPhoneNumber);
    smsPhoneNumber.trim();
    smsMessage.trim();
    smsMessage.toUpperCase();
    
    String upperUntriggerSecretKey = UNTRIGGER_KEY;
    if (smsMessage == upperUntriggerSecretKey && smsPhoneNumber == PHONE_NUMBER){
      untrigger();
    }

    String upperTriggerSecretKey = TRIGGER_KEY;
    if (smsMessage == upperTriggerSecretKey && smsPhoneNumber == PHONE_NUMBER){
      alarm();
    }
    
    hasSMS = false;
  }
}

void buttonPressedInt()
{
  buttonPressed = true;
}

void movedInt()
{
  moved = true;
}

void beep(int interval, byte times)
{
  for (byte i = 0; i < times; ++i)
  {
    siren.turnOn();
    delay(interval);
    siren.turnOff();
    delay(interval);
  }
}

void alarm()
{
  beep(BIP_INTERVAL, 1);
  alarmTime = millis();
  alarmed = true;
}

void unAlarm()
{
  beep(BIP_INTERVAL, 2);
  alarmed = false;
  preTriggered = false;
  alarmTime = 0;
}

void preTrigger()
{
  setupGSM();
  beep((BIP_INTERVAL * 2), 1);
  preTriggered = true;
  preTriggerTime = millis();
}

void trigger()
{
  siren.turnOn();
  makeCall(PHONE_NUMBER);

  triggered = true;
  triggerTime = millis();
  lastCallTime = millis();
}

void untrigger()
{
  siren.turnOff();
  delay(100);
  beep(BIP_INTERVAL, 3);
  rejectCall();

  preTriggered = false;
  triggered = false;
  alarmed = false;
  alarmTime = 0;
  callAccepted = false;
  stopCallingPasswordInput = "";
}

void setupGSM() {
   serialGSM.print("AT+CMGF=1\n;AT+CNMI=2,2,0,0,0\n;ATX4\n;AT+COLP=0\n;AT+DDET=1\n;"); 
}

void makeCall(String phoneNumber) {
  serialGSM.println("ATH0\n");
  serialGSM.print((char)26); 
  serialGSM.println("ATD " + phoneNumber + ";\n");
  serialGSM.print((char)26); 
}

void rejectCall()
{
  serialGSM.println("ATH0");
  serialGSM.println((char)26);
}

void readGSM()
{
  static String recText = "";
  static unsigned long delay1 = 0;
  static int count = 0;
  static unsigned char buffer[64];

  if (serialGSM.available())
  {
    while (serialGSM.available())
    {
      buffer[count++] = serialGSM.read();
      if (count == 64)
        break;
    }

    recText += (char *)buffer;
    delay1 = millis();

    for (int i = 0; i < count; i++)
    {
      buffer[i] = NULL;
    }
    count = 0;
  }

  if (((millis() - delay1) > 100) && recText != "")
  {
    if (recText.substring(2, 7) == "+CMT:")
    {
      hasSMS = true;
    }

    if (hasSMS)
    {
      smsPhoneNumber = "";
      smsMessage = "";

      byte line = 0;
      byte qMarks = 0;
      for (int nL = 1; nL < recText.length(); nL++)
      {
        if (recText.charAt(nL) == '"')
        {
          qMarks++;
          continue;
        }
        if ((line == 1) && (qMarks == 1))
        {
          smsPhoneNumber += recText.charAt(nL);
        }
        if (line == 2)
        {
          smsMessage += recText.charAt(nL);
        }
        if (recText.substring(nL - 1, nL + 1) == "\r\n")
        {
          line++;
        }
      }
    }
    else
    {
      gsmCommand = recText;
    }
    recText = "";
  }
}
