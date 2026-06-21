#include <stdint.h>
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


/*
 * If we subdivide 1 step into 256 subdivisions we can measure microsteps
 * in 256th steps. This enum helps with knowing what how far the motor advances
 * for each microstep setting. Notice that it is the inverse of the enum in
 * StepperMotorLib.h
 */
typedef enum: uint16_t
{
   Step256 = 1,
   Step128 = 2,
   Step64 = 4,
   Step32 = 8,
   Step16 = 16,
   Step8 = 32,
   Step4 = 64,
   Step2 = 128,
   Step1 = 256
}MicroStepSizeEnum;

//MotorState Enum
typedef enum: uint8_t
{
   M_INIT,
   M_HOME,
   M_HOMING,
   M_EXTENDING,
   M_EXTENDED,
   M_GOING_TO,
   M_AT_POSITION
}MotorStateEnum;

typedef struct
{
  //Int that is 0-199 that represents major steps
  uint8_t posMajorStep;
  //int that is is 0-255 that represents microsteps that make up a major step
  uint8_t posMinorStep;
}MotorPosition;

//Compare Two Motor Positions, 
// if ptr1 > ptr2 return 1
// if ptr1 == ptr2 return 0
// if ptr1 < ptr2 return -1
int8_t compareMotorPosition( MotorPosition * pos1, MotorPosition * pos2 )
{
  if( pos1->posMajorStep == pos2->posMajorStep )
  {
    if( pos1->posMinorStep == pos2->posMinorStep )
    {
      return 0;
    }
    else if( pos1->posMinorStep > pos2->posMinorStep )
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else if( pos1->posMajorStep > pos2->posMajorStep )
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

//Motor Position Controlling Struct
typedef struct
{
  //toggles true when the home limitSwitch is found
  bool homeFound = false;
  //is true when the limit switch is pressed
  bool isHome;
  //is true when the position is at the software limit
  bool isAtLimit = false;
  //isStopped
  bool isStopped = true;
  //Position Struct
  MotorPosition Position;
  //Target Position for the motor to go to
  MotorPosition Target;
  //Software limit for the maximum extension of the motor unit is decidegrees
  uint16_t extendedLimDecidegrees;
  //Software Limit but in steps and microsteps
  MotorPosition extendedLimitRaw;
  //Speed of the motor in decidegrees per second
  uint16_t speedDeciDegrees;
  //Struct that represents the limit switch
  Button HomeLimitSwitch;
  //Struct that represents the motor wrapper
  StepperMotor StepMotor;
  //Remembers the microstepping mode setting
  //also useful for counting microsteps per macrostep
  MicroStepSizeEnum MicroStepSize;
  DirectionNameEnum HomingDirection;
  DirectionNameEnum ExtendingDirection;
  DirectionNameEnum Direction;
  MotorStateEnum MotorState = M_INIT;
}StepperController;

//Sets the microstepping mode
//Cannot be run before Motor Init!
void controllerSetMicroStepMode(
   StepperController * Controller,
   MicroStepModeEnum MicroStepMode
   )
{
   //Set MicroStep Size
   switch(MicroStepMode)
  {
    case MicroStep1: Controller->MicroStepSize=Step1; break;
    case MicroStep2: Controller->MicroStepSize=Step2; break;
    case MicroStep4: Controller->MicroStepSize=Step4; break;
    case MicroStep8: Controller->MicroStepSize=Step8; break;
    case MicroStep16: Controller->MicroStepSize=Step16; break;
    case MicroStep32: Controller->MicroStepSize=Step32; break;
    case MicroStep64: Controller->MicroStepSize=Step64; break;
    case MicroStep128: Controller->MicroStepSize=Step128; break;
    case MicroStep256: Controller->MicroStepSize=Step256; break;
  }
  //MotorWrapperLib
  setMicroStepParameter( &(Controller->StepMotor), MicroStepMode);
}

//Sets the Limit Position and extendedLimDecidegrees members
void controllerSetLimitAngle( StepperController * Controller, uint16_t angleLimitDeciDegrees)
{
  Controller->extendedLimDecidegrees = angleLimitDeciDegrees;
  uint8_t majorStep = angleLimitDeciDegrees / 18;
  uint8_t minorStep = (angleLimitDeciDegrees % 18 )*256/18;
  Controller->extendedLimitRaw.posMajorStep = majorStep;
  Controller->extendedLimitRaw.posMinorStep = minorStep;
}

//Init
void stepperControllerInit(
  StepperController * Controller,
  StepperMotorPinNames * MotorPinNames,
  MicroStepModeEnum MicroStepMode,
  DirectionNameEnum HomingDirection,
  uint8_t limitSwitchPin,
  uint16_t motorCurrent,
  uint16_t motorSpeed,
  uint16_t limitAngleDeciDegrees
)
{
  //Set Controller member variables
  Controller->HomingDirection = HomingDirection;
  //Set Antihoming direction
  if(HomingDirection == AntiClockwise)
  {
    Controller->ExtendingDirection = Clockwise;
  }
  else 
  {
    Controller->ExtendingDirection = AntiClockwise;
  }

  //Direction Starts Out Heading Towards Home
  Controller->Direction = HomingDirection;
  Controller->speedDeciDegrees = motorSpeed;

  //Set Limit angle
  controllerSetLimitAngle( Controller, limitAngleDeciDegrees);
  
  //Home LimitSwitch Has not been found, so motor state is unknown
  //All we know is that it's somewhere between home and max extension
  Controller->MotorState = M_INIT;

  
  //init the motor
  //InitDirection to Homing Direction
  stepperMotorInit(
    &(Controller->StepMotor),
    MotorPinNames,
    motorCurrent,
    motorSpeed,
    MicroStepMode,
    HomingDirection
  );

  //Set Microstepmode for motor and microstepsize for controller
  controllerSetMicroStepMode( Controller, MicroStepMode);

  //init the limit switch
  buttonInit( &( Controller->HomeLimitSwitch ), limitSwitchPin );
}

//Find Home

//Go Home

void controllerEnable( StepperController * Controller )
{
  Controller->isStopped = false;
}

void controllerDisable( StepperController * Controller )
{
  Controller->isStopped = true;
}


//IsHome ?
bool controllerIsHome( StepperController * Controller )
{
   bool isHome = false;
   //Check if button is active
   if( Controller->HomeLimitSwitch.active == true )
   {
      //debounce the switch
      if( debounce( &(Controller->HomeLimitSwitch) ) )
      {
        Serial.println("Home found");
         isHome = true;
      }
   }
   return isHome;
}

//Set Speed
void controllerSetSpeed( StepperController * Controller, uint16_t speedDeciDegrees )
{
  setSpeed( &( Controller->StepMotor ), speedDeciDegrees );
}

//Set Direction
void controllerSetDirection( StepperController * Controller, DirectionNameEnum Direction)
{
  Controller->Direction = Direction;
  setDirection( &( Controller->StepMotor ), Direction );
}

void controllerSetState( StepperController * Controller, MotorStateEnum MotorState)
{
  Serial.println("Change state " + String(MotorState));
  Controller->MotorState = MotorState;
}

//Set Zero
//Sets the zero point of the motor, use when isHome == true
void controllerSetZero( StepperController * Controller)
{
  Controller->homeFound = true;
  Controller->Position.posMajorStep = 0;
  Controller->Position.posMinorStep = 0;
}

//Increment Position
void controllerIncrementPosition( StepperController * Controller)
{
  //capture position
  uint8_t posMinorLast = Controller->Position.posMinorStep;
  uint8_t posMajorLast = Controller->Position.posMajorStep;
  
  //negative. going towards home
  if( Controller->Direction == Controller->HomingDirection )
  {
    Controller->Position.posMinorStep -= Controller->MicroStepSize;
    //detect minor step underflow
    if( Controller->Position.posMinorStep >= posMinorLast )
    {
      Controller->Position.posMinorStep -= 1;
      //detect major step underflow
      //Shouldn't happen on zoltaur
      if( Controller->Position.posMajorStep >= 200 )
      {
        Controller->Position.posMajorStep = 199;
      }
    }
  }
  //positive, going away from home
  else //Controller->Direction == AwayFromHome
  {
    Controller->Position.posMinorStep += Controller->MicroStepSize;
    //detectrollover
    if( Controller->Position.posMinorStep <= posMinorLast )
    {
      Controller->Position.posMajorStep += 1;
      //Shouldn't happen on Zoltaur, but Major Rollover
      if ( Controller->Position.posMajorStep >= 200 )
      {
        Controller->Position.posMajorStep = 0;
      }
    }
  }
}

//Step
//increments position if home is known
//Anticlockwise is negative
//Clockwise is positive
void controllerStep( StepperController * Controller)
{
  //step returns a bool and steps the motor
  //so capture it to know what to do with incrementing
  bool hasStepped = step( &(Controller->StepMotor) );
  //if has stepped has completed a cycle and controller has found home
  if( hasStepped && Controller->homeFound )
  {
   controllerIncrementPosition( Controller );
  }
}

//get position raw
//returns a pointer to the position struct
MotorPosition * controllerGetPositionRaw( StepperController * Controller )
{
  return &( Controller->Position );
}

//get position in deci degrees
uint16_t controllerGetPositionDeciDeg( StepperController * Controller)
{
  //18 Decidegrees per major step
  uint16_t positionMajor = Controller->Position.posMajorStep * 18;
  //128 minor steps is 9 decidegrees
  uint16_t positionMinor = ( Controller->Position.posMinorStep * 18 ) / 256 ;

  return positionMajor+positionMinor;
}

//IsAtLimit
bool controllerIsAtLimit( StepperController * Controller)
{
  MotorPosition * posPtr = &( Controller->Position );
  MotorPosition * limPtr = &( Controller->extendedLimitRaw );
  if( compareMotorPosition(posPtr, limPtr) >= 0 )
  {
    return true;
  }
  return false;
}

//Go to position absolute


//Go To position relative

//UpdateMotor
//Statemachine for motor position
void updateMotor(StepperController * Controller)
{
  switch( Controller->MotorState)
  {
    //Start Up State
    case M_INIT:
      //If At home, set Zero, Direction to clockwise to extend, and setstate to home
      if(controllerIsHome(Controller))
      {
        controllerSetZero( Controller );
        controllerSetDirection( Controller, Controller->ExtendingDirection );
        controllerSetState( Controller, M_HOME);
      }
      else // else step the controller towards where home should be
      {
        controllerStep( Controller );
      }
      break;

    case M_HOME:
      //If isStopped is false then the motor does things
      if(!Controller->isStopped)
      {
        controllerSetDirection( Controller, Controller->ExtendingDirection );
        controllerSetState( Controller, M_EXTENDING);
        break;
      }
      else
      {
        break;
      }

    case M_HOMING:
      //If At home, set Zero, Direction to clockwise to extend, and setstate to home
      if(controllerIsHome(Controller))
      {
        controllerSetZero( Controller );
        controllerSetDirection( Controller, Controller->ExtendingDirection );
        controllerSetState( Controller, M_HOME);
      }
      else // else step the controller towards where home should be
      {
        controllerStep( Controller );
      }
      break;

    //Increment towards extended position
    case M_EXTENDING:
      if( controllerIsAtLimit( Controller ) )
      {
        controllerSetState( Controller, M_EXTENDED );
        break;
      }
      else
      {
        controllerStep( Controller );
        break;
      }

    //At Extended Position
    case M_EXTENDED:
      controllerSetDirection( Controller, Controller->HomingDirection );
      controllerSetState( Controller, M_HOMING );
      break;

    //Heading To Position
    case M_GOING_TO:
      switch( compareMotorPosition( &( Controller->Position ), &( Controller->Target ) ) )
      {
        //If Positions are equal then go to M_AT_POSITION
        case 0:
          controllerSetState( Controller, M_AT_POSITION );
          break;
        //If motor Position is above  Target
        case 1:
          controllerSetDirection( Controller, Controller->HomingDirection );
          controllerStep( Controller );
          break;
        //If motor Position is below Target
        case -1:
          controllerSetDirection( Controller, Controller->ExtendingDirection );
          controllerStep( Controller );
          break;
      }
      break;

    case M_AT_POSITION:
      break;
  }
}

#endif /* STEPPERCONTROLLERLIB_H*/