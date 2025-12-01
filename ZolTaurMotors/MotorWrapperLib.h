/*
 * 
 * Aut
 * 
 * Library for controlling motor settings.
 */

 #include <SPI.h>
 #include "src/HighPowerStepperDriver/HighPowerStepperDriver.h"

typedef enum 
{
  DirPin = 2,
  StepPin = 3,
  CsPin = 4,
}Pin_Type;

typedef enum
{
   forward, //Counter Clockwise
   reverse  //Clockwise
}Direction_Type;

typedef enum 
{
   ForwardSpeed = 80,
   ReverseSpeed = 8000
}Rpmx100_Type;

uint32_t perConverter(uint32_t shaft10xRpm)
{
   uint32_t stepPeruS = 0;
   
   uint32_t mGearRatio = 46646; //1000x gear ratio
   
   stepPeruS = 1000000 * 1000 /128 * 60 / 200 / shaft10xRpm * 100 / mGearRatio;
   return stepPeruS;
}

typedef struct
{
   uint8_t DirPin;
   uint8_t StepPin;
   uint8_t CsPin;
   uint32_t fStepPerioduS;
   uint32_t rStepPerioduS;
   HighPowerStepperDriver sd;
}mtrWrap;



//Inits the motor from library
void motorInit
( 
   mtrWrap *motor,
   Rpmx100_Type speed,
   Pin_Type pin,
   Direction_Type Dir 
)
{
   //Set pins
   motor->DirPin = (pin = DirPin);
   motor->StepPin = (pin = StepPin);
   motor->CsPin = ( pin = CsPin);
   
   //init motor driver
   SPI.begin();
   motor->sd.setChipSelectPin(motor->CsPin);
   
   
   // Drive the STEP and DIR pins low initially.
   pinMode(motor->StepPin, OUTPUT);
   digitalWrite(motor->StepPin, LOW);
   pinMode(motor->DirPin, OUTPUT);
   digitalWrite(motor->DirPin, LOW);
   
   // Give the driver some time to power up.
   delay(1);
   
   // Reset the driver to its default settings and clear latched status
   // conditions.
   motor->sd.resetSettings();
   motor->sd.clearStatus();
   
   // Select auto mixed decay.  TI's DRV8711 documentation recommends this mode
   // for most applications, and we find that it usually works well.
   motor->sd.setDecayMode(HPSDDecayMode::AutoMixed);
   
   // Set the current limit. You should change the number here to an appropriate
   // value for your particular system.
   motor->sd.setCurrentMilliamps36v4(4000);
   
   // Set the number of microsteps that correspond to one full step.
   motor->sd.setStepMode(HPSDStepMode::MicroStep128);

   // Enable the motor outputs.
   motor->sd.disableDriver();
   
   motor->fStepPerioduS = perConverter(speed = ForwardSpeed);
   motor->rStepPerioduS = perConverter(speed = ReverseSpeed);
}

void setDirection(mtrWrap *motor, Direction_Type Dir)
{
   delayMicroseconds(1);
   digitalWrite(motor->DirPin, Dir);
   delayMicroseconds(1);
}

void step(mtrWrap *motor)
{
   // The STEP minimum high pulse width is 1.9 microseconds.
  digitalWrite(motor->StepPin, HIGH);
  delayMicroseconds(3);
  digitalWrite(motor->StepPin, LOW);
  delayMicroseconds(3);
}

void fStep(mtrWrap *motor)
{
   step(motor);
   delayMicroseconds(motor->fStepPerioduS);
}

void rStep(mtrWrap *motor)
{
   step(motor);
   delayMicroseconds(motor->rStepPerioduS);
}



