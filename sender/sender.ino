#define CODE_BYTES  4
#define BYTE_SIZE 8
#define BIN_CODE_SIZE 32
#define NOT_FOUND -1
#define ON 1
#define OFF 0

int IRledPin =  13;
int pulseLength = 0;

int pulseValues[CODE_BYTES];
//                            0 Y Y Y Y Y Y Y 0 P P P P P P P C T T T T T T T 0 A A A A A A A
int binCode[BIN_CODE_SIZE] = {0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

const char SYMBOLS_ON[BIN_CODE_SIZE]  = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','!','@','#','$','%','^'};
const char SYMBOLS_OFF[BIN_CODE_SIZE] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6'};

void setup()
{
  pinMode(IRledPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  sendCode();
  /*
  char x = Serial.read();
  if(x == 'U')
    digitalWrite(IRledPin, HIGH);
  if(x == 'u')
    digitalWrite(IRledPin, LOW);
  */
}

void sendCode()
{
  checkPulseChanges();
  
  pulseIR(2000);
  delayMicroseconds(2000);
  pulseLength=4000;
  
  for(int i = 0; i < BIN_CODE_SIZE; i++)
  {
    sendPulseValue(binCode[i]);
  }
  
  pulseIR(300);
  delayMicroseconds((26560 - pulseLength));
}

void checkPulseChanges()
{
  char inCtrlByte = Serial.read();
  int index;
  index = indexOf(inCtrlByte, SYMBOLS_ON);
  if(index != NOT_FOUND)
  {
    binCode[index] = ON;
  }
  else
  {
    index = indexOf(inCtrlByte, SYMBOLS_OFF);
    if(index != NOT_FOUND)
    {
      binCode[index] = OFF;
    }
  }
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

void sendPulseValue(int pulseValue)
{
  if (pulseValue == ON)
  {
    pulseIR(300);
    delayMicroseconds(600);
    pulseLength += 900;
  }
  else if(pulseValue == OFF)
  {
    pulseIR(300);
    delayMicroseconds(300);
    pulseLength += 600;
  }
}

int indexOf(char c, const char cArr[])
{
  boolean found = false;
  int index = 0;
  while(!found && index < BIN_CODE_SIZE)
  {
    found = (cArr[index] == c);
    index++;
  }
  if(found)
  {
    index--;
    return index;
  }
  else
  {
    return NOT_FOUND;
  }
}
