#include "LeapListener.h"
#include <vcclr.h>
using namespace Leap;
using namespace System;
using namespace std;

gcroot<SerialPort^> arduino;
gcroot<String^> portName;
int baudRate;

const int ON = 1;
const int OFF = 0;
const int CODE_SIZE = 32;

const int YAW_SHIFT = 0;

const string PITCH = "pitch";
const int PITCH_SHIFT = 8;
const int PITCH_SCALAR = -50;
const float PITCH_SENSITIVITY = 0.25f;
int curr_pitch,prev_pitch,pitch_change = 63;

const string THROTTLE = "throttle";
const int THROTTLE_SHIFT = 16;
const int THROTTLE_SCALAR = 200;
int curr_throttle,prev_throttle,throttle_change = 0;

                                               //0   Y   Y   Y   Y   Y   Y   Y   0   P   P   P   P   P   P   P   C   T   T   T   T   T   T   T   0   A   A   A   A   A   A   A
const gcroot<String^> SYMBOLS_ON[CODE_SIZE]  = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","!","@","#","$","%","^"};
const gcroot<String^> SYMBOLS_OFF[CODE_SIZE] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","1","2","3","4","5","6"};

gcroot<String^> changeChar;

void LeapListener::onInit(const Controller& controller)
{
  Console::WriteLine("Initialized");
}

void LeapListener::onConnect(const Controller& controller)
{
  portName = "COM3";
  baudRate = 9600;
  arduino = gcnew SerialPort(portName, baudRate);
  arduino->Open();
  Console::WriteLine("Connected");
}

void LeapListener::onDisconnect(const Controller& controller)
{
  arduino->Close();
  Console::WriteLine("Disconnected");
}

void LeapListener::onExit(const Controller& controller)
{
  Console::WriteLine("Exited");
}

static int checkBounds(int value)
{
  if( value > 127)
  {
    return  127;
  }
  else if (value < 0)
  {
    return  0;
  }
  return value;
}

static void normalizeThrottle(float current_throttle)
{
  curr_throttle += ((int)(current_throttle) / THROTTLE_SCALAR);
  curr_throttle = checkBounds(curr_throttle);
}

static void normalize_pitch(float current_pitch)
{
  curr_pitch = (int)(current_pitch * PITCH_SCALAR);
  if(curr_pitch < (prev_pitch * (1+PITCH_SENSITIVITY)) && curr_pitch > (prev_pitch * (1-PITCH_SENSITIVITY)))
  {
    curr_pitch = prev_pitch;
  }
  curr_pitch = checkBounds(curr_pitch);
}

vector<int> * binArray(int num)
{
  vector<int> * ret = new vector<int>(8,0);
  int my_num = num;
  if(my_num < 0)
    my_num *= -1;
  int index = 7;
  while (my_num != 0 && index >= 0)
  {
    ret->at(index) = my_num % 2;
    my_num /= 2;
    index--;
  }
  return ret;
}

String^ onOffSymbol(int index, bool on)
{
  if(on)
  {
    return SYMBOLS_ON[index];
  }
  else
  {
    return SYMBOLS_OFF[index];
  }
}

String^ getCharacter(string section, int index, bool on)
{
  if(section == THROTTLE)
  {
    return onOffSymbol(index + THROTTLE_SHIFT, on);
  }
  else if(section == PITCH)
  {
    return onOffSymbol(index + PITCH_SHIFT, on);
  }
  return "";
}

void checkChangeSerialWrite(vector<int> * value_vector, vector<int> * change_vector, string section, int index)
{
  if(change_vector->at(index) == 1)
  {
    changeChar = getCharacter(section, index, value_vector->at(index) == ON);
    if(changeChar->Length != 0)
    {
      arduino->WriteLine(changeChar);
    }
  }
}

void LeapListener::onFrame(const Controller& controller)
{
  HandList hands = controller.frame().hands();
  if(!hands.empty())
  {
    normalizeThrottle(hands[0].palmVelocity().y);
    float raw_curr_pitch = hands[0].palmNormal().pitch();
    normalize_pitch(raw_curr_pitch);

    throttle_change = prev_throttle ^ curr_throttle;
    pitch_change = prev_pitch ^ curr_pitch;

    vector<int> * throttle_vector = binArray(curr_throttle);
    vector<int> * throttle_change_vector = binArray(throttle_change);
    vector<int> * pitch_vector = binArray(curr_pitch);
    vector<int> * pitch_change_vector = binArray(pitch_change);

    int index = 1;
    while(index < 8)
    {
      checkChangeSerialWrite(throttle_vector, throttle_change_vector, THROTTLE, index);
      checkChangeSerialWrite(pitch_vector, pitch_change_vector, PITCH, index);
      index++;
    }
    
    prev_throttle = curr_throttle;
    prev_pitch = curr_pitch;
    delete throttle_vector;
    delete throttle_change_vector;
    delete pitch_vector;
    delete pitch_change_vector;
  }
}