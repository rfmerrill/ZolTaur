/*
 * Pawprint Prototyping:
 */

#include <SPI.h>
#include "MotorWrapperLib.h"
#include "Button.h"

enum State
{
  Init,
  Stby 
  //wave, talk, wave2, etc
};

volatile uint8_t intFlag = 0; //interrupt flags

uint8_t stateVar;
mtrWrap armMotor, mouthMotor;
Rpmx100_Type r;
Pin_Type p; //might need refactoring
Direction_Type d;
Button_Type b; //might need refactoring

Button button1, button2; //etc

int ledPin = 9;
volatile bool ledState = true;



void setup()
{
  // put your setup code here, to run once:
  stateVar = 0;
  //init led
  pinMode(ledPin, OUTPUT);

  //init buttons
  //buttonInit();
}

void loop()
{
  // put your main code here, to run repeatedly:

}
