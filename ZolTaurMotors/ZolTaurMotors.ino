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
#include "StepperControllerLib.h"
#include "Button.h"

// State Stuff ///////////////////
//Enum for easy state name access in switch case later
typedef enum
{
  INIT,
  STBY,
  GOHOME,
  HOME,
  GOTODEGREE,
  TurnClockwise,
  TurnAntiClockwise,
  Wave1,
  Wave2
  //wave, talk, wave2, etc
}TopState; 

volatile uint32_t intFlag = 0; //interrupt flags
//Create the state variable for switch case later
//uint8_t stateVar;
TopState StateVar;

//Debug pin
uint8_t debugPin = 20;


//Motors ///////////////////////

//Declare Arm Motor
//StepperMotor armMotor;
//StepperMotor * armMotorPtr = &armMotor;
//Declare Arm Motor Pins
StepperMotorPinNames ArmMotorPins = { .directionPin = 2, .stepPin = 3, .chipSelectPin = 4};
StepperMotorPinNames * ArmPinsPtr = &ArmMotorPins;
//Arm Microstepping Mode
MicroStepModeEnum StepModeEnum = MicroStep64;
//Arm speed in tenths of a degree per second
uint16_t armSpeed = 100;
//Arm Per phase current
uint16_t armCurrent = 1800;
//Arm limit in deci degrees. 300 == 30 degrees
uint16_t armLimitDeciDeg = 300;

//Declare Jaw Motor
//StepperMotor jawMotor;
//StepperMotor * jawMotorPtr = &jawMotor;
//Declare Jaw motor pins
StepperMotorPinNames JawMotorPins = { .directionPin = 5, .stepPin = 6, .chipSelectPin = 7};
StepperMotorPinNames * JawPinsPtr = &JawMotorPins;
//Jaw Speed
uint16_t jawSpeed = 900;
uint16_t jawCurrent = 1800;
uint16_t jawLimitDeciDeg = 300;
MicroStepModeEnum JawStepEnum = MicroStep64;

//Declare Motor Controller
//Arm
StepperController ArmController;
StepperController * ArmControlPtr = &ArmController;

//Jaw
StepperController JawController;
StepperController * JawControlPtr = &JawController;

//Buttons //////////////////////

//Button ArmHomeLimitSwitch, ArmOpenLimitSwitch; //arm limit switches
//Button JawHomeLimitSwitch;
//Button EmergencyStopBttn; // eStopState
//Button Pointers
//Note Compiler refuses to let me declare multiple pointers of a type button *a,*b,*c etc
// and set them later, so I have to set them as I create them
//Makes things more readable later
//Button *EmergencyStopBttnPtr = &EmergencyStopBttn;
//Button *ArmHomeLimSwPtr = &ArmHomeLimitSwitch;
//Button *ArmOpenLimSwPtr = &ArmOpenLimitSwitch;
//Button *JawHomeLimSwPtr = &JawHomeLimitSwitch;



//Button mode; //debug button for changing mode, remove after integrating rPi talk

//Input Pin numbers
uint8_t  EmergencyStopBttnPin = 8;
uint8_t  ArmHmLimSwPin = 9;
uint8_t  ArmOpenLimSwPin = 10;
uint8_t  JawHmLimSwPin = 11;


//uint8_t ledPin = LED_BUILTIN;
//volatile bool ledState = true;



void setup()
{
  // put your setup code here, to run once:
  //StateMachine starts at init, which finishes initializing
  //anything that didn't finish here
  StateVar = INIT;
  
  //Set Debug Pin
  pinMode(debugPin, INPUT);

  //init buttons
  //buttonInit(EmergencyStopBttnPtr, EmergencyStopBttnPin);
  //buttonInit(ArmHomeLimSwPtr, ArmHmLimSwPin);
  //buttonInit(ArmOpenLimSwPtr, ArmOpenLimSwPin);
  //buttonInit(JawHomeLimSwPtr, JawHmLimSwPin);

  //init motors
  //Set their CS Pins to low
  pinMode(ArmPinsPtr->chipSelectPin, OUTPUT);
  digitalWrite(ArmPinsPtr->chipSelectPin, LOW);
  pinMode(JawPinsPtr->chipSelectPin, OUTPUT);
  digitalWrite(JawPinsPtr->chipSelectPin, LOW);

//Init Motor Controllers:
  //Arm
  stepperControllerInit(
    ArmControlPtr,
    ArmPinsPtr,
    StepModeEnum,
    Clockwise,
    ArmHmLimSwPin,
    armCurrent,
    armSpeed,
    armLimitDeciDeg);
  //Jaw
  stepperControllerInit(
    JawControlPtr,
    JawPinsPtr,
    StepModeEnum,
    AntiClockwise,
    JawHmLimSwPin,
    jawCurrent,
    jawSpeed,
    jawLimitDeciDeg);

  //arm
  /*
  stepperMotorInit(
    armMotorPtr,
    armPinsPtr,
    armCurrent,
    armSpeed,
    armStepEnum,
    AntiClockwise);
  //jaw
  stepperMotorInit(
    jawMotorPtr,
    jawPinsPtr,
    jawCurrent,
    jawSpeed,
    jawStepEnum,
    AntiClockwise);
*/

  //attach interrupts
  //attachInterrupt(digitalPinToInterrupt(EmergencyStopBttn.pinNumber), emergencyStopBttnISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ArmController.HomeLimitSwitch.pinNumber), armHomeLimSwISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(ArmOpenLimitSwitch.pinNumber), armOpenLimSwISR, RISING);
  attachInterrupt(digitalPinToInterrupt(JawController.HomeLimitSwitch.pinNumber), jawHomeLimSwISR, RISING);
}

void loop()
{
  // put your main code here, to run repeatedly:
  //step(armMotorPtr);
  if( digitalRead( debugPin ) )
  {
    updateMotor(ArmControlPtr);
    updateMotor(JawControlPtr);
  }
  else 
  {
  
  }
  //top level state machine
  /*
  switch(StateVar)
  {
    case INIT:
      //Init
      break;

    case STBY:
      //Standby, don't move
      break;

    case GOHOME:
      //Home all Motors
      break;

    case HOME:
      //Motors are at home;
      break;
  }
  */
}

//here be ISR's


void armHomeLimSwISR()
{
  ArmController.HomeLimitSwitch.active = true;
}


void jawHomeLimSwISR()
{
  JawController.HomeLimitSwitch.active = true;
}