//Library for integrating position sensing homing, and movement patterns
/*
 * Written by Autunite
 */

#ifndef STEPPERCONTROLLERLIB_H
#define STEPPERCONTROLLERLIB_H

#include "StepperMotorLib.h"
#include "Button.h"

/*
 * This Library integratesa stepper motor with one or two Limit Switches
 * StepperControllers are structs that tracks and controls a stepper motor's position
 * Positions can be relative or absolute depending on whether the home position has been found
 *
 */

//Motor Position Controlling Struct
typedef struct
{
  //toggles true when the home limitSwitch is found
  bool homeFound = false;
  //is true when the limit switch is pressed
  bool isHome;
  //is true when the position is at the software limit
  bool isAtLimit = false;
  //Int that is 0-200 that represents major steps
  uint8_t posMajorStep;
  //int that is is 0-255 that represents microsteps that make up a major step
  uint8_t postMinorStep;
  //Software limit for the maximum extension of the motor unit is decidegrees
  uint16_t extendedLimDecidegrees;
  //position of the motor in decidegrees
  uint16_t posDeciDegrees;
  //Speed of the motor in decidegrees per second
  uint16_t speedDeciDegrees;
  //Struct that represents the limit switch
  Button HomeLimitSwitch;
  //Struct that represents the motor wrapper
  StepperMotor StepMotor;
  //Remembers the microstepping mode setting
  //also useful for counting microsteps per macrostep
  MicroStepModeEnum MicroStepMode;
}StepperController;

//Init
void StepperControllerInit(
  StepperController * Controller,
  StepperMotorPinNames * MotorPinNames,
  MicroStepModeEnum MicroStepMode,
  DirectionNameEnum HomingDirection,
  uint8_t limitSwitchPin,
  uint16_t motorCurrent,
  uint16_t motorSpeed
)
{
  //Set Controller member variables
  
  //init the motor
  stepperMotorInit(
    &(Controller->StepMotor),
    MotorPinNames,
    motorCurrent,
    motorSpeed,
    MicroStepMode,
    HomingDirection
  );

  //init the limit switch
  buttonInit( &(Controller->HomeLimitSwitch), limitSwitchPin);
}

//Find Home

//Go Home

//IsHome

//IsAtLimit

//Set Speed

//Go to position absolute


//Go To position relative




#endif /* STEPPERCONTROLLERLIB_H*/