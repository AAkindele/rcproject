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

const int PITCH_SHIFT = 8;

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

static void normalize_throttle(float current_throttle)
{
  curr_throttle += ((int)(current_throttle) / THROTTLE_SCALAR);
  if( curr_throttle > 127)
  {
    curr_throttle =  127;
  }
  else if (curr_throttle < 0)
  {
    curr_throttle =  0;
  }
}

vector<int> * bin_array(int num)
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

String^ get_character(string section, int index, bool on)
{
  if(section == THROTTLE)
  {
    if(on)
    {
      return SYMBOLS_ON[index + THROTTLE_SHIFT];
    }
    else
    {
      return SYMBOLS_OFF[index + THROTTLE_SHIFT];
    }
  }
  return "";
}

void LeapListener::onFrame(const Controller& controller)
{
  HandList hands = controller.frame().hands();
  if(!hands.empty())
  {
    normalize_throttle(hands[0].palmVelocity().y);
    throttle_change = prev_throttle ^ curr_throttle;

    vector<int> * throttle_vector = bin_array(curr_throttle);
    vector<int> * throttle_change_vector = bin_array(throttle_change);

    //index 0 for this byte is the channel. start at index 1. counting from left to right for these vectors
    int index = 1;
    while(index < 8)
    {
      //there is a change at this index
      if(throttle_change_vector->at(index) == 1)
      {
        changeChar = get_character(THROTTLE, index, throttle_vector->at(index) == ON);
        if(changeChar->Length != 0)
        {
          arduino->WriteLine(changeChar);
        }
      }
      index++;
    }
    
    prev_throttle = curr_throttle;
    delete throttle_vector;
    delete throttle_change_vector;
  }
}