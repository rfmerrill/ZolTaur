 /*
 * 
 * Aut
 * 
 * Library for controlling stepper motor direction, speed, microstepping ratio, 
 * and provides a step function that provides feedback on when a step completes.
 * Also provides a handy way to convert speed values in decidegrees per second
 * to half the stepper period.
 * 
 * This is for the TM2208 Stepper Driver
 * Since microstepping is set by hardware, the microstepping for this struct
 * is for internal position reference
 */
 #ifndef StepperMotorLib_TMC2208_H
 #define StepperMotorLib_TMC2208_H

 //#include <SPI.h>
 //#include "src/HighPowerStepperDriver/HighPowerStepperDriver.h"

/*Definitions:
//MacroStep: 
  One full magnetic step, felt when turning the motor while its off
  usually every 1.8 degrees on common motors or 200 steps per 360 degrees
//Microstep:
  A setting for the driver so that it can hold it's position at subdivisions of the
  macro step, used for finer control.
  Be careful when changing microstep settings as we need to write to the driver with spi
  So maybe only do it when the motor is stopped
//DeciDegri/TenthDegree:
  A tenth of a degree
*/
//StepMode Enumerated Type
//ie 256 steps makes one full step
typedef enum : uint16_t
{
  MicroStep256_TMC2208 = 256,
  MicroStep128_TMC2208 = 128,
  MicroStep64_TMC2208  =  64,
  MicroStep32_TMC2208  =  32,
  MicroStep16_TMC2208  =  16,
  MicroStep8_TMC2208   =   8,
  MicroStep4_TMC2208   =   4,
  MicroStep2_TMC2208   =   2,
  MicroStep1_TMC2208   =   1
}MicroStepModeEnum_TMC2208;

//Directions enumerated type
typedef enum : uint8_t
{
  AntiClockwise_TMC2208, //Counter Clockwise
  Clockwise_TMC2208  //Clockwise
}DirectionNameEnum_TMC2208;


//Use this struct so you can set you motor pins in one nameable spot
typedef struct
{
  uint8_t directionPin; //Direction Pin
  uint8_t stepPin; //Step Pin
  //uint8_t chipSelectPin; // Chip Select Pin
}StepperMotorPinNames_TMC2208;

//had to use lower case because of name collisions with macros
typedef enum : uint8_t{
  rising_TMC2208,
  high_TMC2208,
  falling_TMC2208,
  low_TMC2208
}StepStateEnum_TMC2208;



//Struct that defines the stepper motor values and settings
typedef struct
{
  uint8_t directionPin; //Direction Pin
  uint8_t stepPin; //Step Pin
  //uint8_t chipSelectPin; // Chip Select Pin
  //uint16_t motorCurrent; //MotorCurrent in MilliAmperes
  //Speed in degrees per second
  uint16_t motorSpeedTenthsDegPerSec;
  //Period in MicroSeconds for StepPinToggle derived from MotorSpeedDegPerSec
  uint32_t uStepHalfPeriodPerMicroStep;
  //Stepper Direction AntiClockwise or Clockwise
  DirectionNameEnum_TMC2208 Direction;
  MicroStepModeEnum_TMC2208 MicroStepMode;
  //Stepper Driver from HighPowerStepperDriver.h
  //HighPowerStepperDriver StepDriver;
  //Timer for the stepper period
  uint32_t stepTimer;
  StepStateEnum_TMC2208 StepState;
}StepperMotor_TMC2208;

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
uint32_t perConverter_TMC2208( StepperMotor_TMC2208 * StepMotor, uint16_t speedTenthsDegreePerSecond )
{
  const uint8_t degTenthsPerStep = 18; //1.8 degrees per step
  const uint32_t million = 1000000;
  uint16_t uStepMode = StepMotor-> MicroStepMode;
  uint16_t speed = speedTenthsDegreePerSecond;
  uint32_t uSecondsPerUStep = (uint32_t)(degTenthsPerStep*million)/(uStepMode*speed);
  return uSecondsPerUStep;
}

//Helper Functions /////////////////////
//Since this doesn't use SPI, this just reflects a hardware value on the
// TMC2208
void setMicroStepParameter_TMC2208( StepperMotor_TMC2208 * StepMotor, MicroStepModeEnum_TMC2208 MicroStepMode )
{
  StepMotor->MicroStepMode = MicroStepMode;
}

void setDirection_TMC2208(StepperMotor_TMC2208 * StepMotor, DirectionNameEnum_TMC2208 Direction)
{
  StepMotor->Direction = Direction;
  digitalWrite(StepMotor->directionPin, StepMotor->Direction);
}

////Caution, don't call this function until the motor has had it's
//microstepping parameters have been set.
//Takes in a value representing tenths of a degree per second and sets the
//TODO: Consider the and plan around the ramifications of setting the speed
//to zero (stopping the motor) and how to prevent issues from that
void setSpeed_TMC2208( StepperMotor_TMC2208 * StepMotor, uint16_t speedTenthDegPerSec )
{
  StepMotor->motorSpeedTenthsDegPerSec = speedTenthDegPerSec;
  StepMotor->uStepHalfPeriodPerMicroStep=perConverter_TMC2208(StepMotor, speedTenthDegPerSec);
}

//Same caution and requirements as setSpeed
uint16_t getSpeed_TMC2208(StepperMotor_TMC2208 * StepMotor)
{
  return StepMotor->motorSpeedTenthsDegPerSec;
}

//Inits the motor from library
void stepperMotorInit_TMC2208(
  StepperMotor_TMC2208 * StepMotor, //Pointer to stepper Motor Struct
  StepperMotorPinNames_TMC2208 * PinNames,
  //uint16_t motorCurrent,
  uint16_t motorSpeed,
  MicroStepModeEnum_TMC2208 MicroStepMode,
  DirectionNameEnum_TMC2208 Direction
)
{
  
  //Set pins
  StepMotor->directionPin = PinNames->directionPin;
  StepMotor->stepPin = PinNames->stepPin;
  //StepMotor->chipSelectPin = PinNames->chipSelectPin;

  //init motor driver
  //SPI.begin();
  //StepMotor->StepDriver.setChipSelectPin(StepMotor->chipSelectPin);

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
  //StepMotor->StepDriver.resetSettings();
  //StepMotor->StepDriver.clearStatus();

  // Select auto mixed decay.  TI's DRV8711 documentation recommends this mode
  // for most applications, and we find that it usually works well.
  //StepMotor->StepDriver.setDecayMode(HPSDDecayMode::AutoMixed);

  //Set Per Phase Current
  //StepMotor->motorCurrent = motorCurrent;
  //StepMotor->StepDriver.setCurrentMilliamps36v4(StepMotor->motorCurrent);

  //Set microstep fraction parameter (Number of MicroSteps Per Full Step)
  setMicroStepParameter_TMC2208(StepMotor, MicroStepMode);

  //Set Motor Speed
  StepMotor->motorSpeedTenthsDegPerSec = motorSpeed;

  //Set microstep half Period in microseconds
  setSpeed_TMC2208(StepMotor, motorSpeed);

  //Set Motor Direction
  setDirection_TMC2208(StepMotor, Direction);

//Set StepState to rising (first state)
  StepMotor->StepState = rising_TMC2208;

  //Enable Output Driver
  //StepMotor->StepDriver.enableDriver();
}



//call this repeatedly to step the motor, the function will return false 
//until a full step is completed
bool step_TMC2208( StepperMotor_TMC2208 * StepMotor )
{
  // The STEP minimum high pulse width is 1.9 microseconds.
  switch (StepMotor->StepState)
  {
    //raise step pin and start timer, goto high
    case rising_TMC2208:
      digitalWrite(StepMotor->stepPin, HIGH);
      StepMotor->stepTimer = micros();
      StepMotor->StepState = high_TMC2208;
      return false;
    // check period timer then go to falling when timer == period
    case high_TMC2208:
      //rollover safe currentTime - startTime >=Period
      if(micros()- StepMotor->stepTimer >= StepMotor->uStepHalfPeriodPerMicroStep)
        {
          StepMotor->StepState = falling_TMC2208;
          return false;
        }
      else
        return false;
    //lower step pin and start timer, goto low
    case falling_TMC2208:
      digitalWrite(StepMotor->stepPin, LOW);
      StepMotor->stepTimer = micros();
      StepMotor->StepState = low_TMC2208;
      return false;
    //check timer and then goto rising and return true to signify the end of a cycle
    //then goto rising
    case low_TMC2208:
      //rollover safecomparison
      if(micros()-StepMotor->stepTimer >= StepMotor->uStepHalfPeriodPerMicroStep)
        {
          StepMotor->StepState = rising_TMC2208;
          return true;
        }
      else
        return false;
    //start at rising as a default if state gets lost for whatever reason
    default:
      StepMotor->StepState = rising_TMC2208;
      return false;
  }
}

#endif /* StepperMotorLib_TMC2208_H */