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
StepperMotor * armMotorPtr = &armMotor;
//Declare Arm Motor Pins
StepperMotorPinNames armMotorPins = { .directionPin = 2, .stepPin = 3, .chipSelectPin = 4};
StepperMotorPinNames * armPinsPtr = &armMotorPins;
//Arm Microstepping Mode
MicroStepModeEnum armStepEnum = MicroStep64;
//Arm speed in tenths of a degree per second
uint16_t armSpeed = 100;
//Arm Per phase current
uint16_t armCurrent = 2100;

//Declare Jaw Motor
StepperMotor jawMotor;
StepperMotor * jawMotorPtr = &jawMotor;
//Declare Jaw motor pins
StepperMotorPinNames jawMotorPins = { .directionPin = 5, .stepPin = 6, .chipSelectPin = 7};
StepperMotorPinNames * jawPinsPtr = &jawMotorPins;
//Jaw Speed
uint16_t jawSpeed = 100;
uint16_t jawCurrent = 1000;
MicroStepModeEnum jawStepEnum = MicroStep64;

//Buttons //////////////////////

Button ArmHomeLimitSwitch, ArmOpenLimitSwitch; //arm limit switches
Button JawHomeLimitSwitch;
Button EmergencyStopBttn; // eStopState
//Button Pointers
//Note Compiler refuses to let me declare multiple pointers of a type button *a,*b,*c etc
// and set them later, so I have to set them as I create them
//Makes things more readable later
Button *EmergencyStopBttnPtr = &EmergencyStopBttn;
Button *ArmHomeLimSwPtr = &ArmHomeLimitSwitch;
Button *ArmOpenLimSwPtr = &ArmOpenLimitSwitch;
Button *JawHomeLimSwPtr = &JawHomeLimitSwitch;



//Button mode; //debug button for changing mode, remove after integrating rPi talk

//name access for button pins
//note: LimSw == limit switch
//using an enum to prevent number collisions of the pins if the list gets long
typedef enum : uint8_t
{
  EmergencyStopBttnPin = 8,
  ArmHmLimSwPin = 9,
  ArmOpenLimSwPin = 10,
  JawHmLimSwPin = 11
}ButtonNameBundleType;

//uint8_t ledPin = LED_BUILTIN;
//volatile bool ledState = true;



void setup()
{
  // put your setup code here, to run once:
  stateVar = Init;
  

  //init buttons
  buttonInit(EmergencyStopBttnPtr, EmergencyStopBttnPin);
  buttonInit(ArmHomeLimSwPtr, ArmHmLimSwPin);
  buttonInit(ArmOpenLimSwPtr, ArmOpenLimSwPin);
  buttonInit(JawHomeLimSwPtr, JawHmLimSwPin);

  //init motors
  //Set their CS Pins to low
  pinMode(armPinsPtr->chipSelectPin, OUTPUT);
  digitalWrite(armPinsPtr->chipSelectPin, LOW);
  pinMode(jawPinsPtr->chipSelectPin, OUTPUT);
  digitalWrite(jawPinsPtr->chipSelectPin, LOW);

  //arm
  StepperMotorInit(
    armMotorPtr,
    armPinsPtr,
    armCurrent,
    armSpeed,
    armStepEnum,
    AntiClockwise);
  //jaw
  StepperMotorInit(
    jawMotorPtr,
    jawPinsPtr,
    jawCurrent,
    jawSpeed,
    jawStepEnum,
    AntiClockwise);

  //attach interrupts
  attachInterrupt(digitalPinToInterrupt(EmergencyStopBttn.PinNum), emergencyStopBttnISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ArmHomeLimitSwitch.PinNum), armHomeLimSwISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ArmOpenLimitSwitch.PinNum), armOpenLimSwISR, RISING);
  attachInterrupt(digitalPinToInterrupt(JawHomeLimitSwitch.PinNum), jawHomeLimSwISR, RISING);
}

void loop()
{
  // put your main code here, to run repeatedly:
  step(armMotorPtr);
}

//here be ISR's
void emergencyStopBttnISR()
{
  EmergencyStopBttn.active = true;
}

void armHomeLimSwISR()
{
  ArmHomeLimitSwitch.active = true;
}

void armOpenLimSwISR()
{
  ArmOpenLimitSwitch.active = true;
}

void jawHomeLimSwISR()
{
  JawHomeLimitSwitch.active = true;
}