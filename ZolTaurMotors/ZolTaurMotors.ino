/*
 * Pawprint Prototyping:
 * 
 * Arduino Due
 * Libraries Used:
 * HiTorque Motor:
 * https://github.com/pololu/high-power-stepper-driver-arduino
 * DueTimer:
 * https://github.com/ivanseidel/DueTimer
 */

#include <SPI.h>
#include "StepperMotorLib.h"
#include "Button.h"

// State Stuff ///////////////////
//Enum for easy state name access in switch case later
enum State
{
  Init,
  Stby,
  Home,
  goToDegree,
  TurnClockwise,
  TurnAntiClockwise,
  Wave1,
  Wave2
  //wave, talk, wave2, etc
};

volatile uint32_t intFlag = 0; //interrupt flags
//Create the state variable for switch case later
uint8_t stateVar;



//Motors ///////////////////////

//Struct for default stepper motor 0 pin names


//Declare Arm Motor
StepperMotor armMotor;




//Buttons //////////////////////

Button limitSwHome, limitSwitchOpen; //etc
Button mode; //debug button for changing mode, remove after integrating rPi talk

//name access for button pins
typedef enum
{
   HmLimSwPin = 5,
   OpenLimSw1Pin = 6,
   modeBttnPin = 7
   //sBtnPin = 8
}Button_Name_Bundle_Type;

Button_Name_Bundle_Type BttnNmBndl;

uint8_t ledPin = LED_BUILTIN;
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
