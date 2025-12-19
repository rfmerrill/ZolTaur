 /*
 * 
 * Aut
 * 
 * Library for controlling stepper motor position, direction, speed.
 */
 
 #include <SPI.h>
 #include "src/HighPowerStepperDriver/HighPowerStepperDriver.h"

/*Definitions:
//MacroStep: 
  One full magnetic step, felt when turning the motor while its off
  usually every 1.8 degrees on common motors or 200 steps per 360 degrees
//Microstep:
  A setting for the driver so that it can hold it's position at subdivisions of the
  macro step, used for finer control.
  Be careful when changing microstep settings as we need to write to the driver with spi
  So maybe only do it when the motor is stopped
*/
//StepMode Enumerated Type
//ie 256 steps makes one full step
typedef enum
{
  MicroStep256 = 256,
  MicroStep128 = 128,
  MicroStep64  =  64,
  MicroStep32  =  32,
  MicroStep16  =  16,
  MicroStep8   =   8,
  MicroStep4   =   4,
  MicroStep2   =   2,
  MicroStep1   =   1
}MicroStepModeEnum;

//Directions enumerated type
typedef enum
{
  AntiClockwise, //Counter Clockwise
  Clockwise  //Clockwise
}DirectionNameEnum;









//Struct that defines the stepper motor values and settings
typedef struct
{
  uint8_t directionPin; //Direction Pin
  uint8_t stepPin; //Step Pin
  uint8_t chipSelectPin; // Chip Select Pin
  uint16_t motorCurrent; //MotorCurrent in MilliAmperes
  //Speed in degrees per second
  uint16_t motorSpeedTenthsDegPerSec;
  //Period in MicroSeconds for StepPinToggle derived from MotorSpeedDegPerSec
  uint32_t uStepHalfPeriodPerMicroStep;
  //Stepper Direction AntiClockwise or Clockwise
  DirectionNameEnum Direction;
  MicroStepModeEnum MicroStepMode;
  //Stepper Driver from HighPowerStepperDriver.h
  HighPowerStepperDriver StepDriver;
}StepperMotor;

//period or frequency conversion here
//Convert tenths of a degree per second to the pulse period in microseconds
//needed to make the motor achieve that speed. Takes into account the
//microstepping mode of the motor. Also this assumes that the motor turns
//1.8 degrees or 18 tenths of a degree per full step
//This function takes in a speed that's in tenths of a degree per second
//and it returns a number measured in microseconds that corresponds to half the
//motors full pulse width
//Decidegrees are used so that integer division can be avoided
//Caution, don't call this function until the motor has had it's dd/sec speed
//and it's microstepping setting have been set
uint32_t perConverter(StepperMotor *StepMotor, uint16_t speedTenthsDegreePerSecond)
{
  const uint8_t degTenthsPerStep = 18; //1.8 degrees per step
  const uint32_t million = 1000000;
  uint16_t uStepMode = StepMotor-> MicroStepMode;
  uint16_t speed = speedTenthsDegreePerSecond;
  uint32_t uSecondsPerUStep = (uint32_t)(degTenthsPerStep*million)/(uStepMode*speed);
  return uSecondsPerUStep;
}

//Helper Functions /////////////////////
//Slow since it calls on SPI
void setMicroStepParameter(StepperMotor *StepMotor, MicroStepModeEnum MicroStepMode)
{
  StepMotor->MicroStepMode = MicroStepMode;
  switch (StepMotor->MicroStepMode)
  {
    case MicroStep1: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep1); break;
    case MicroStep2: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep2); break;
    case MicroStep4: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep4); break;
    case MicroStep8: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep8); break;
    case MicroStep16: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep16); break;
    case MicroStep32: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep32); break;
    case MicroStep64: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep64); break;
    case MicroStep128: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep128); break;
    case MicroStep256: StepMotor->StepDriver.setStepMode(HPSDStepMode::MicroStep256); break;
 }
}

void setdirection(StepperMotor *StepMotor, DirectionNameEnum Direction)
{
  StepMotor->Direction = Direction;
  digitalWrite(StepMotor->directionPin, StepMotor->Direction);
}

////Caution, don't call this function until the motor has had it's
//microstepping parameters have been set.
//Takes in a value representing tenths of a degree per second and sets the
//TODO: Consider the and plan around the ramifications of setting the speed
//to zero (stopping the motor) and how to prevent issues from that
void setSpeed(StepperMotor *StepMotor, uint16_t speedTenthDegPerSec)
{
  StepMotor->motorSpeedTenthsDegPerSec = speedTenthDegPerSec;
  StepMotor->uStepHalfPeriodPerMicroStep=perConverter(StepMotor, speedTenthDegPerSec);
}

//Same caution and requirements as setSpeed
uint16_t getSpeed(StepperMotor *StepMotor)
{
  return StepMotor->motorSpeedTenthsDegPerSec;
}

//Inits the motor from library
void StepperMotorInit
(
  StepperMotor *StepMotor, //Pointer to stepper Motor Struct
  uint8_t dirPin,
  uint8_t stepPin,
  uint8_t chipselectPin,
  uint16_t motorCurrent,
  uint16_t motorSpeed,
  MicroStepModeEnum MicroStepMode,
  DirectionNameEnum Direction
)
{
  //Set pins
  StepMotor->directionPin = dirPin;
  StepMotor->stepPin = stepPin;
  StepMotor->chipSelectPin = chipselectPin;

  //init motor driver
  SPI.begin();
  StepMotor->StepDriver.setChipSelectPin(StepMotor->chipSelectPin);

  // Drive the STEP and DIR pins low initially.
  pinMode(StepMotor->stepPin, OUTPUT);
  pinMode(StepMotor->directionPin, OUTPUT);
  digitalWrite(StepMotor->stepPin, LOW);
  digitalWrite(StepMotor->directionPin, LOW);

  // Give the driver some time to power up.
  uint32_t initMotorDelayTimeStamp  = millis();
  while(millis()-initMotorDelayTimeStamp<1){}

  // Reset the driver to its default settings and clear latched status
  // conditions.
  StepMotor->StepDriver.resetSettings();
  StepMotor->StepDriver.clearStatus();

  // Select auto mixed decay.  TI's DRV8711 documentation recommends this mode
  // for most applications, and we find that it usually works well.
  StepMotor->StepDriver.setDecayMode(HPSDDecayMode::AutoMixed);

  //Set Per Phase Current
  StepMotor->motorCurrent = motorCurrent;
  StepMotor->StepDriver.setCurrentMilliamps36v4(StepMotor->motorCurrent);

  //Set microstep fraction parameter (Number of MicroSteps Per Full Step)
  setMicroStepParameter(StepMotor, MicroStepMode);

  //Set Motor Speed
  StepMotor->motorSpeedTenthsDegPerSec = motorSpeed;

  //Set microstep half Period in microseconds
  setSpeed(StepMotor, motorSpeed);

  //Set Motor Direction
  setdirection(StepMotor, Direction);

  //Enable Output Driver
  StepMotor->StepDriver.enableDriver();
}


void step(StepperMotor *StepMotor)
{
  // The STEP minimum high pulse width is 1.9 microseconds.
  
}

