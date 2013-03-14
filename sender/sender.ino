int IRledPin =  13;
int pulseValues[32] = {0,0,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1,1,1,1};
int pulseLength = 0;

void setup()
{
  pinMode(IRledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("sending ir codes!");
}

void loop()                     
{
  //SendCode();
}

void SendCode()
{
  pulseIR(2000);
  delayMicroseconds(2000);
  pulseLength=4000;
  
  
  for(int i = 0; i < 32; i++)
  {
    sendPulseValue(pulseValues[i]);
  }
  
  pulseIR(300);
  delayMicroseconds((26560 - pulseLength));
}

void pulseIR(long microsecs)
{
  cli();
  while (microsecs > 0)
  {
    digitalWrite(IRledPin, HIGH);
    delayMicroseconds(9);
    digitalWrite(IRledPin, LOW);
    delayMicroseconds(9);

    microsecs -= 24;
  }
  sei();
}

void sendPulseValue(int pulseValue)
{
  if (pulseValue == 1)
    One();
  else
    Zero();
}

void Zero()
{
  pulseIR(300);
  delayMicroseconds(300);
  pulseLength += 600;
}

void One()
{
  pulseIR(300);
  delayMicroseconds(600);
  pulseLength += 900;
}
