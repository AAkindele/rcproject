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

const string YAW = "yaw";
int curr_yaw,prev_yaw,yaw_change = 63;

const string PITCH = "pitch";
const int PITCH_SHIFT = 8;
//const int PITCH_SCALAR = -50;
//const float PITCH_SENSITIVITY = 0.25f;
int curr_pitch,prev_pitch,pitch_change = 63;

const string THROTTLE = "throttle";
const int THROTTLE_SHIFT = 16;
const int THROTTLE_SCALAR = 100;
int curr_throttle,prev_throttle,throttle_change = 0;

                                               //0   Y   Y   Y   Y   Y   Y   Y   0   P   P   P   P   P   P   P   C   T   T   T   T   T   T   T   0   A   A   A   A   A   A   A
const gcroot<String^> SYMBOLS_ON[CODE_SIZE]  = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","!","@","#","$","%","^"};
const gcroot<String^> SYMBOLS_OFF[CODE_SIZE] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","1","2","3","4","5","6"};

gcroot<String^> changeChar;

HandList hands;

void LeapListener::onInit(const Controller& controller)
{
  Console::WriteLine("Initialized");
}

void LeapListener::onConnect(const Controller& controller)
{
  portName = "COM8";
  baudRate = 9600;
  arduino = gcnew SerialPort(portName, baudRate);
  arduino->ReadTimeout = 1;
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

static void normalizePitch(float current_pitch)
{
  if(-(current_pitch) < 0.54)
  {
    curr_pitch = 13;
  }
  else if(-(current_pitch) < 1.08)
  {
    curr_pitch = 38;
  }
  else if(-(current_pitch) < 1.62)
  {
    curr_pitch = 63;
  }
  else if(-(current_pitch) < 2.16)
  {
    curr_pitch = 89;
  }
  else if(-(current_pitch) < 2.7)
  {
    curr_pitch = 114;
  }
}

static void normalizeYaw(float current_yaw)
{
  if(current_yaw >= -1 && current_yaw <= 1)
  {
    curr_yaw = 63;
  }
  else if(current_yaw < -1)
  {
    curr_yaw = 31;
  }
  else if(current_yaw > 1)
  {
    curr_yaw = 94;
  }
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
  else if(section == YAW)
  {
    return onOffSymbol(index,on);
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
  hands = controller.frame().hands();
  if(!hands.empty())
  {
	  normalizeThrottle(hands[0].palmVelocity().y);
    normalizePitch(hands[0].palmNormal().pitch());
    normalizeYaw(hands[0].palmNormal().yaw());

    throttle_change = prev_throttle ^ curr_throttle;
    pitch_change = prev_pitch ^ curr_pitch;
    yaw_change = prev_yaw ^ curr_yaw;

    vector<int> * throttle_vector = binArray(curr_throttle);
    vector<int> * throttle_change_vector = binArray(throttle_change);
    vector<int> * pitch_vector = binArray(curr_pitch);
    vector<int> * pitch_change_vector = binArray(pitch_change);
    vector<int> * yaw_vector = binArray(curr_yaw);
    vector<int> * yaw_change_vector = binArray(yaw_change);

    int index = 1;
    while(index < 8)
    {
      checkChangeSerialWrite(throttle_vector, throttle_change_vector, THROTTLE, index);
      checkChangeSerialWrite(pitch_vector, pitch_change_vector, PITCH, index);
      checkChangeSerialWrite(yaw_vector, yaw_change_vector, YAW, index);
      index++;
    }

    cout << curr_throttle << " " << curr_pitch << " " << curr_yaw << endl;

    prev_throttle = curr_throttle;
    prev_pitch = curr_pitch;
    prev_yaw = curr_yaw;
    delete throttle_vector;
    delete throttle_change_vector;
    delete pitch_vector;
    delete pitch_change_vector;
    delete yaw_vector;
    delete yaw_change_vector;

    try
    {
      arduino->ReadByte();
    }
    catch(System::TimeoutException^)
    {
    }
  }
}