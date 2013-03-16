#define CODE_BYTES  4
#define BYTE_SIZE 8
#define BIN_CODE_SIZE 32

int IRledPin =  13;
int pulseLength = 0;

//yaw and pitch dont use the 8th bit. its always 0
//channel is the 8th bit on the throttle byte.
int yawIndex = 0; // controls rotation. 0 is max left. 127 is max right. 63 is center
int pitchIndex = 1; // controls foward and backwards movement. 0 is full tilt back. 127 is full tilt foward. 63 is center
int throttleIndex = 2; // controls throttle/altitude. 0 is min, 127 is max
int yawAdjustIndex = 3;

int channel = 128; // The channel is technically 1. Since its the 8th in the byte, the value is (2^7)=128. This will be or'ed with the throttle value to generate the 3rd byte.

int pulseValues[CODE_BYTES];
//                            0 Y Y Y Y Y Y Y 0 P P P P P P P C T T T T T T T 0 A A A A A A A
int binCode[BIN_CODE_SIZE] = {0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int inCtrlByte = 0;   // what control variable to change and which direction

void setup()
{
  pinMode(IRledPin, OUTPUT);
  Serial.begin(9600);
  pulseValues[yawIndex] = 63;
  pulseValues[pitchIndex] = 63;
  pulseValues[throttleIndex] = 0; // initially the throttle is zero. The throttle value will be or'ed with the channel value to generate the throttle byte
  pulseValues[yawAdjustIndex] = 0;
}

void loop()
{/*
  //sendCode();
  //checkPulseChanges();
  for(int i = 0; i < 32; i++)
  {
    Serial.print(binCode[i]);
    if(i % 8 == 7)
      Serial.print(" ");
  }
  Serial.println("");
  //delay(2000);*/
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
  inCtrlByte = Serial.read();
  int start = BIN_CODE_SIZE;
  int low = 0;
  int high = 127;
  int tmp = 0;
  int bin[BYTE_SIZE] = {0,0,0,0,0,0,0,0};
  if(inCtrlByte == 'Y' || inCtrlByte == 'y')
  {
    if(inCtrlByte == 'Y' && pulseValues[yawIndex] < high)
    {
      pulseValues[yawIndex]++;
    }
    else if(inCtrlByte == 'y' && pulseValues[yawIndex] > low)
    {
      pulseValues[yawIndex]--;
    }
    
    //from index 0 to 7
    start = 0;
    decToBin(pulseValues[yawIndex], bin);
  }
  else if(inCtrlByte == 'P' || inCtrlByte == 'p')
  {
    if(inCtrlByte == 'P' && pulseValues[pitchIndex] < high)
    {
      pulseValues[pitchIndex]++;
    }
    else if(inCtrlByte == 'p' && pulseValues[pitchIndex] > low)
    {
      pulseValues[pitchIndex]--;
    }
    
    //from index 8 to 15
    start = 8;
    decToBin(pulseValues[pitchIndex], bin);
  }
  else if(inCtrlByte == 'T' || inCtrlByte == 't')
  {
    if(inCtrlByte == 'T' && pulseValues[throttleIndex] < high)
    {
      pulseValues[throttleIndex]++;
    }
    else if(inCtrlByte == 't' && pulseValues[throttleIndex] > low)
    {
      pulseValues[throttleIndex]--;
    }
    
    //from index 16 to 23
    start = 16;
    //channel or'ed with throttle gives the third byte in the frame
    decToBin(pulseValues[throttleIndex] | channel, bin);
  }
  
  //copy the changed byte into the correct section of the frame
  //use the start variable as the of the changed byte
  int finish = start + BYTE_SIZE;
  for(int i = start; (i < finish && i < BIN_CODE_SIZE); i++)
  {
    binCode[i] = bin[i - start];
  }
}

void decToBin(int dec, int bin[8])
{
  int tmp = dec;
  int index = 7;
  while (tmp != 0 && index >= 0)
  {
    bin[index] = tmp % 2;
    tmp /= 2;
    index--;
  }
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
