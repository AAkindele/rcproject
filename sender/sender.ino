#include "ir_codes.h"

int IRledPin = 13;

void setup()
{
  pinMode(IRledPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  Serial.println("Sending IR signal");
  sendCode();
  delay(60*1000);
}

void pulseIR(long microsecs)
{
  cli();
  while (microsecs > 0)
  {
    digitalWrite(IRledPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(IRledPin, LOW);
    delayMicroseconds(10);
    microsecs -= 26;
  }
  sei();
}

void sendCode()
{
  for(int i = 0; i < PCODE_SIZE; i++)
  {
    if(i % 2 == 0)
    {
      pulseIR(power_code[i]);
    }
    else
    {
      delayMicroseconds(power_code[i]);
    }
  }
}
