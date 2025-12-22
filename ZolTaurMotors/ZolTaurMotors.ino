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




//Declare Arm Motor
StepperMotor armMotor;
StepperMotor * armMotorPointer = &armMotor;
//Declare Arm Motor Pins
StepperMotorPinNames armMotorPins = { .directionPin = 2, .stepPin = 3, .chipSelectPin = 4};
StepperMotorPinNames * armPinsPointer = &armMotorPins;
//Arm Microstepping Mode
MicroStepModeEnum armStepEnum = MicroStep64;
//Arm speed in tenths of a degree per second
uint16_t armSpeed = 100;
//Arm Per phase current
uint16_t armCurrent = 2100;

//Declare Jaw Motor
StepperMotor jawMotor;
StepperMotor * jawMotorPointer = &jawMotor;
//Declare Jaw motor pins
StepperMotorPinNames jawMotorPins = { .directionPin = 5, .stepPin = 6, .chipSelectPin = 7};
StepperMotorPinNames * jawPinsPointer = &jawMotorPins;
//Jaw Speed
uint16_t jawSpeed = 100;
uint16_t jawCurrent = 1000;
MicroStepModeEnum jawStepEnum = MicroStep64;

//Buttons //////////////////////

Button limitSwHome, limitSwitchOpen; //etc
Button mode; //debug button for changing mode, remove after integrating rPi talk

//name access for button pins
typedef enum : uint8_t
{
   HmLimSwPin = 5,
   OpenLimSw1Pin = 6,
   modeBttnPin = 7
   //sBtnPin = 8
}Button_Name_Bundle_Type;

Button_Name_Bundle_Type BttnNmBndl;

//uint8_t ledPin = LED_BUILTIN;
//volatile bool ledState = true;



void setup()
{
  // put your setup code here, to run once:
  stateVar = Init;
  

  //init buttons
  //buttonInit();

  //attach interrupts

  //init motors
  //Set their CS Pins to low
  pinMode(armPinsPointer->chipSelectPin, OUTPUT);
  digitalWrite(armPinsPointer->chipSelectPin, LOW);
  pinMode(jawPinsPointer->chipSelectPin, OUTPUT);
  digitalWrite(jawPinsPointer->chipSelectPin, LOW);

  //arm
  StepperMotorInit(armMotorPointer, armPinsPointer, 2100, 100, armStepEnum , AntiClockwise);
  //jaw
  StepperMotorInit(jawMotorPointer, jawPinsPointer, 2100, 100, jawStepEnum , AntiClockwise);



}

void loop()
{
  // put your main code here, to run repeatedly:
  step(armMotorPointer);
}
