#define IRpin_PIN PIND
#define IRpin 2
#define RESOLUTION 20
#define NUMPULSES 100

#include "ir_codes.h"

void setup(void) {
  Serial.begin(9600);
  Serial.println("Ready to decode IR!");
}

uint16_t pulses[NUMPULSES][2];
uint8_t currentpulse = 0;

void loop(void) {
  uint16_t highpulse, lowpulse;
  highpulse = lowpulse = 0;
  
  while (IRpin_PIN & (1 << IRpin)) {
    highpulse++;
    delayMicroseconds(RESOLUTION);
  }
  pulses[currentpulse][0] = highpulse;
  while (! (IRpin_PIN & _BV(IRpin))) {
     lowpulse++;
     delayMicroseconds(RESOLUTION);
  }
  pulses[currentpulse][1] = lowpulse;
  
  currentpulse++;
  
  if(currentpulse == NUMPULSES) {
    currentpulse = 0;
  }
}

void printpulses(void) {
  Serial.println("\n\r\n\rReceived: \n\rOFF \tON");
  for (uint8_t i = 0; i < currentpulse; i++) {
    Serial.print(pulses[i][0] * RESOLUTION, DEC);
    Serial.print(" usec, ");
    Serial.print(pulses[i][1] * RESOLUTION, DEC);
    Serial.println(" usec");
  }
}
