//Library for integrating position sensing homing, and movement patterns
/*
 * Written by Autunite
 */

#ifndef STEPPERCONTROLLERLIB_TMC2208_H
#define STEPPERCONTROLLERLIB_TMC2208_H

#include <stdint.h>
#include "StepperMotorLib_TMC2208.h"
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
   Step256_TMC2208 = 1,
   Step128_TMC2208 = 2,
   Step64_TMC2208 = 4,
   Step32_TMC2208 = 8,
   Step16_TMC2208 = 16,
   Step8_TMC2208 = 32,
   Step4_TMC2208 = 64,
   Step2_TMC2208 = 128,
   Step1_TMC2208 = 256
}MicroStepSizeEnum_TMC2208;

//MotorState Enum
typedef enum: uint8_t
{
   M_INIT_TMC2208,
   M_HOME_TMC2208,
   M_HOMING_TMC2208,
   M_EXTENDING_TMC2208,
   M_EXTENDED_TMC2208,
   M_GOING_TO_TMC2208,
   M_AT_POSITION_TMC2208
}MotorStateEnum_TMC2208;

typedef struct
{
  //Int that is 0-199 that represents major steps
  uint8_t posMajorStep;
  //int that is is 0-255 that represents microsteps that make up a major step
  uint8_t posMinorStep;
}MotorPosition_TMC2208;

//Compare Two Motor Positions, 
// if ptr1 > ptr2 return 1
// if ptr1 == ptr2 return 0
// if ptr1 < ptr2 return -1
int8_t compareMotorPosition_TMC2208( MotorPosition_TMC2208 * pos1, MotorPosition_TMC2208 * pos2 )
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
  MotorPosition_TMC2208 Position;
  //Target Position for the motor to go to
  MotorPosition_TMC2208 Target;
  //Software limit for the maximum extension of the motor unit is decidegrees
  uint16_t extendedLimDecidegrees;
  //Software Limit but in steps and microsteps
  MotorPosition_TMC2208 extendedLimitRaw;
  //Speed of the motor in decidegrees per second
  uint16_t speedDeciDegrees;
  //Struct that represents the limit switch
  Button HomeLimitSwitch;
  //Struct that represents the motor wrapper
  StepperMotor_TMC2208 StepMotor;
  //Remembers the microstepping mode setting
  //also useful for counting microsteps per macrostep
  MicroStepSizeEnum_TMC2208 MicroStepSize;
  DirectionNameEnum_TMC2208 HomingDirection;
  DirectionNameEnum_TMC2208 ExtendingDirection;
  DirectionNameEnum_TMC2208 Direction;
  MotorStateEnum_TMC2208 MotorState = M_INIT_TMC2208;
}StepperController_TMC2208;

//Sets the microstepping mode
//Cannot be run before Motor Init!
void controllerSetMicroStepMode_TMC2208(
   StepperController_TMC2208 * Controller,
   MicroStepModeEnum_TMC2208 MicroStepMode
   )
{
   //Set MicroStep Size
   switch( MicroStepMode )
  {
    case MicroStep1_TMC2208: Controller->MicroStepSize=Step1_TMC2208; break;
    case MicroStep2_TMC2208: Controller->MicroStepSize=Step2_TMC2208; break;
    case MicroStep4_TMC2208: Controller->MicroStepSize=Step4_TMC2208; break;
    case MicroStep8_TMC2208: Controller->MicroStepSize=Step8_TMC2208; break;
    case MicroStep16_TMC2208: Controller->MicroStepSize=Step16_TMC2208; break;
    case MicroStep32_TMC2208: Controller->MicroStepSize=Step32_TMC2208; break;
    case MicroStep64_TMC2208: Controller->MicroStepSize=Step64_TMC2208; break;
    case MicroStep128_TMC2208: Controller->MicroStepSize=Step128_TMC2208; break;
    case MicroStep256_TMC2208: Controller->MicroStepSize=Step256_TMC2208; break;
  }
  //MotorWrapperLib
  setMicroStepParameter_TMC2208( &(Controller->StepMotor), MicroStepMode);
}

//Sets the Limit Position and extendedLimDecidegrees members
void controllerSetLimitAngle_TMC2208( StepperController_TMC2208 * Controller, uint16_t angleLimitDeciDegrees)
{
  Controller->extendedLimDecidegrees = angleLimitDeciDegrees;
  uint8_t majorStep = angleLimitDeciDegrees / 18;
  uint8_t minorStep = (angleLimitDeciDegrees % 18 )*256/18;
  Controller->extendedLimitRaw.posMajorStep = majorStep;
  Controller->extendedLimitRaw.posMinorStep = minorStep;
}

//Init
void stepperControllerInit_TMC2208(
  StepperController_TMC2208 * Controller,
  StepperMotorPinNames_TMC2208 * MotorPinNames,
  MicroStepModeEnum_TMC2208 MicroStepMode,
  DirectionNameEnum_TMC2208 HomingDirection,
  uint8_t limitSwitchPin,
  //uint16_t motorCurrent,
  uint16_t motorSpeed,
  uint16_t limitAngleDeciDegrees
)
{
  //Set Controller member variables
  Controller->HomingDirection = HomingDirection;
  //Set Antihoming direction
  if(HomingDirection == AntiClockwise_TMC2208)
  {
    Controller->ExtendingDirection = Clockwise_TMC2208;
  }
  else 
  {
    Controller->ExtendingDirection = AntiClockwise_TMC2208;
  }

  //Direction Starts Out Heading Towards Home
  Controller->Direction = HomingDirection;
  Controller->speedDeciDegrees = motorSpeed;

  //Set Limit angle
  controllerSetLimitAngle_TMC2208( Controller, limitAngleDeciDegrees);
  
  //Home LimitSwitch Has not been found, so motor state is unknown
  //All we know is that it's somewhere between home and max extension
  Controller->MotorState = M_INIT_TMC2208;

  
  //init the motor
  //InitDirection to Homing Direction
  stepperMotorInit_TMC2208(
    &(Controller->StepMotor),
    MotorPinNames,
    //motorCurrent,
    motorSpeed,
    MicroStepMode,
    HomingDirection
  );

  //Set Microstepmode for motor and microstepsize for controller
  controllerSetMicroStepMode_TMC2208( Controller, MicroStepMode);

  //init the limit switch
  buttonInit( &( Controller->HomeLimitSwitch ), limitSwitchPin );
}

//Find Home

//Go Home

void controllerEnable_TMC2208( StepperController_TMC2208 * Controller )
{
  Controller->isStopped = false;
}

void controllerDisable_TMC2208( StepperController_TMC2208 * Controller )
{
  Controller->isStopped = true;
}


//IsHome ?
bool controllerIsHome_TMC2208( StepperController_TMC2208 * Controller )
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
void controllerSetSpeed_TMC2208( StepperController_TMC2208 * Controller, uint16_t speedDeciDegrees )
{
  setSpeed_TMC2208( &( Controller->StepMotor ), speedDeciDegrees );
}

//Set Direction
void controllerSetDirection_TMC2208( StepperController_TMC2208 * Controller, DirectionNameEnum_TMC2208 Direction)
{
  Controller->Direction = Direction;
  setDirection_TMC2208( &( Controller->StepMotor ), Direction );
}

void controllerSetState_TMC2208( StepperController_TMC2208 * Controller, MotorStateEnum_TMC2208 MotorState)
{
  Serial.println("Change state " + String(MotorState));
  Controller->MotorState = MotorState;
}

//Set Zero
//Sets the zero point of the motor, use when isHome == true
void controllerSetZero_TMC2208( StepperController_TMC2208 * Controller)
{
  Controller->homeFound = true;
  Controller->Position.posMajorStep = 0;
  Controller->Position.posMinorStep = 0;
}

//Increment Position
void controllerIncrementPosition_TMC2208( StepperController_TMC2208 * Controller)
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
void controllerStep_TMC2208( StepperController_TMC2208 * Controller)
{
  //step returns a bool and steps the motor
  //so capture it to know what to do with incrementing
  bool hasStepped = step_TMC2208( &(Controller->StepMotor) );
  //if has stepped has completed a cycle and controller has found home
  if( hasStepped && Controller->homeFound )
  {
   controllerIncrementPosition_TMC2208( Controller );
  }
}

//get position raw
//returns a pointer to the position struct
MotorPosition_TMC2208 * controllerGetPositionRaw_TMC2208( StepperController_TMC2208 * Controller )
{
  return &( Controller->Position );
}

//get position in deci degrees
uint16_t controllerGetPositionDeciDeg_TMC2208( StepperController_TMC2208 * Controller)
{
  //18 Decidegrees per major step
  uint16_t positionMajor = Controller->Position.posMajorStep * 18;
  //128 minor steps is 9 decidegrees
  uint16_t positionMinor = ( Controller->Position.posMinorStep * 18 ) / 256 ;

  return positionMajor+positionMinor;
}

//IsAtLimit
bool controllerIsAtLimit_TMC2208( StepperController_TMC2208 * Controller)
{
  MotorPosition_TMC2208 * posPtr = &( Controller->Position );
  MotorPosition_TMC2208 * limPtr = &( Controller->extendedLimitRaw );
  if( compareMotorPosition_TMC2208(posPtr, limPtr) >= 0 )
  {
    return true;
  }
  return false;
}

//Go to position absolute


//Go To position relative

//UpdateMotor
//Statemachine for motor position
void updateMotor_TMC2208(StepperController_TMC2208 * Controller)
{
  switch( Controller->MotorState)
  {
    //Start Up State
    case M_INIT_TMC2208:
      //If At home, set Zero, Direction to clockwise to extend, and setstate to home
      if( controllerIsHome_TMC2208( Controller ) )
      {
        controllerSetZero_TMC2208( Controller );
        controllerSetDirection_TMC2208( Controller, Controller->ExtendingDirection );
        controllerSetState_TMC2208( Controller, M_HOME_TMC2208);
      }
      else // else step the controller towards where home should be
      {
        controllerStep_TMC2208( Controller );
      }
      break;

    case M_HOME_TMC2208:
      //If isStopped is false then the motor does things
      if(!Controller->isStopped)
      {
        controllerSetDirection_TMC2208( Controller, Controller->ExtendingDirection );
        controllerSetState_TMC2208( Controller, M_EXTENDING_TMC2208);
        break;
      }
      else
      {
        break;
      }

    case M_HOMING_TMC2208:
      //If At home, set Zero, Direction to clockwise to extend, and setstate to home
      if(controllerIsHome_TMC2208(Controller))
      {
        controllerSetZero_TMC2208( Controller );
        controllerSetDirection_TMC2208( Controller, Controller->ExtendingDirection );
        controllerSetState_TMC2208( Controller, M_HOME_TMC2208);
      }
      else // else step the controller towards where home should be
      {
        controllerStep_TMC2208( Controller );
      }
      break;

    //Increment towards extended position
    case M_EXTENDING_TMC2208:
      if( controllerIsAtLimit_TMC2208( Controller ) )
      {
        controllerSetState_TMC2208( Controller, M_EXTENDED_TMC2208 );
        break;
      }
      else
      {
        controllerStep_TMC2208( Controller );
        break;
      }

    //At Extended Position
    case M_EXTENDED_TMC2208:
      controllerSetDirection_TMC2208( Controller, Controller->HomingDirection );
      controllerSetState_TMC2208( Controller, M_HOMING_TMC2208 );
      break;

    //Heading To Position
    case M_GOING_TO_TMC2208:
      switch( compareMotorPosition_TMC2208( &( Controller->Position ), &( Controller->Target ) ) )
      {
        //If Positions are equal then go to M_AT_POSITION
        case 0:
          controllerSetState_TMC2208( Controller, M_AT_POSITION_TMC2208 );
          break;
        //If motor Position is above  Target
        case 1:
          controllerSetDirection_TMC2208( Controller, Controller->HomingDirection );
          controllerStep_TMC2208( Controller );
          break;
        //If motor Position is below Target
        case -1:
          controllerSetDirection_TMC2208( Controller, Controller->ExtendingDirection );
          controllerStep_TMC2208( Controller );
          break;
      }
      break;

    case M_AT_POSITION_TMC2208:
      break;
  }
}

#endif /* STEPPERCONTROLLERLIB_H*/