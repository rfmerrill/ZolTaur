/*
 * Pawprint Prototyping:
 * 
 * Arduino Mega
 * Libraries Used:
 */

#include "SerialController.h"
#include "BasicStepperDriver.h"
#include <string.h>

// anonymous namespace to prevent potential ODR violations
// `constexpr` by default has external linkage, yuck.
namespace {

//Debug pin
constexpr uint8_t debugPin = 21;

//Motors ///////////////////////
// don't use unsigned types for math, see: https://google.github.io/styleguide/cppguide.html#Integer_Types
// Unsigned types have unintuitive semantics and can hide errors (see below).
constexpr int armSpeedDeciDegPerSec = 300;
constexpr int armStepsPerRevolution = 200;
constexpr int armMicrostepsPerStep = 8;

// We need microsteps per second for tone()
// int32_t{} is needed here because otherwise the product overflows
// (this is caught at compile time for SIGNED types only!!)
// but the final result needs to fit in `unsigned int`
// as that's the type of `tone()`'s argument.
constexpr unsigned int armSpeedMicrostepsPerSecond = (armSpeedDeciDegPerSec * int32_t{armStepsPerRevolution} * armMicrostepsPerStep)
                                                     / 3600;
static_assert(armSpeedMicrostepsPerSecond == 133);

//Jaw Speed
constexpr int jawSpeedDeciDegPerSec = 50;
constexpr int jawMoveDeciDeg = 200;
constexpr int jawStepsPerRevolution = 200;
constexpr int jawMicrostepsPerStep = 8;
constexpr int kJawLimitWaitTimeMs = 300;

// BasicStepperDriver library takes float RPM for speed. We match that.
constexpr float jawSpeedRevsPerMin = (jawSpeedDeciDegPerSec * 60.0) / 3600.0;
static_assert(jawSpeedRevsPerMin == 0.83333333333);

//Jaw home angle is how far from home the position target is set to. 
//The goal is to get close to home without setting off the limit switch
constexpr int jawHomeAngle = 25;
// Set this to either 1 or -1
constexpr bool jawDirectionAwayFromLimit = 1;

//Input Pin numbers
constexpr uint8_t EmergencyStopBttnPin = 19;
constexpr uint8_t JawHmLimSwPin = 18;


constexpr uint8_t kArmDirPin = 8;
// Pin 9 is OC2A so we could use timer compare out
// if the tone() library isn't good enough.
constexpr uint8_t kArmStepPin = 9;
constexpr uint8_t kArmSleepPin = 10;

constexpr uint8_t kJawDirPin = 11;
constexpr uint8_t kJawStepPin = 12;
constexpr uint8_t kJawSleepPin = 13;
constexpr uint8_t kJawLimitSwitchPin = 2;

// Serial Parser
SerialParser serialParser;
String outputBuffer;

// Stepper driver for jaw
BasicStepperDriver jawStepper(jawStepsPerRevolution, kJawDirPin, kJawStepPin, kJawSleepPin);

// Did we see a falling edge on the limit switch?
volatile bool jawLimitHit = false;

volatile bool jawRunning = false;
volatile bool armRunning = false;
}

bool isJawReady() {
  return !jawRunning;
}

bool isArmReady() {
  return !armRunning;
}

void setup()
{
  // put your setup code here, to run once:
  
  //Set Debug Pin
  pinMode(debugPin, INPUT);

  //Enable Serial and wait for it to connect.
  Serial.begin(9600);
  while (!Serial) {}

  // Limit switch should be across pin and ground.
  pinMode(kJawLimitSwitchPin, INPUT_PULLUP);

  pinMode(kArmStepPin, OUTPUT);
  pinMode(kArmDirPin, OUTPUT);
  pinMode(kArmSleepPin, OUTPUT);

  // We don't ever change arm dir
  digitalWrite(kArmDirPin, 1);
  // Start awake
  digitalWrite(kArmSleepPin, 1);
  // STEP state doesn't matter.

  //attach interrupts
  attachInterrupt(digitalPinToInterrupt(kJawLimitSwitchPin), jawHomeLimSwISR, RISING);

  jawStepper.begin(jawSpeedRevsPerMin, jawMicrostepsPerStep);
}

// put your main code here, to run repeatedly:
void loop()
{
  // motor updates go here
  updateJawStepper();
  updateSerial();
}


void updateJawStepper() {
  static int direction = 1;
  static bool do_restart_move = true;
  unsigned long wait_until = 0;

  if (jawLimitHit) {
    // TODO: which direction is away from the limit switch?
    direction = jawDirectionAwayFromLimit;
    wait_until = millis() + kJawLimitWaitTimeMs;
    // This will probably get activated more than once due to bounce, just
    // need to make sure we don't start moving until it settles.
    jawLimitHit = false;
  } else if (wait_until > millis()) {
    // pass
  } else if (do_restart_move) {
    do_restart_move = false;
    // Rather than use the float version of this function, pass it
    // decidegrees and then divide by 10 after.
    // Also, don't actually multiply by direction--compiler is too dumb to
    // know it's always 1 or -1.
    long steps;
    if (direction > 0) {
      steps = jawStepper.calcStepsForRotation(long{jawMoveDeciDeg}) / 10;
    } else {
      steps = jawStepper.calcStepsForRotation(long{-jawMoveDeciDeg}) / 10;
    }
    jawStepper.startMove(steps);
  } else {
    // run the next action and see if the commanded movement is done
    bool moveDone = jawStepper.nextAction();
    if (moveDone) {
      // switch directions
      direction = -direction;
      // start a new move
      do_restart_move = true;
    }
  }
}

void updateSerial() {
  if (Serial.available() > 0) {
    int charToRead = Serial.read();
    if (charToRead < 0) return;

    String command = serialParser.parse(charToRead);

    if (command != SerialParser::COMMAND_NONE) {
      String response = "";
      response += command;
      response += " ";

      // --- COMMAND: IS READY? ---
      // ?R
      if (command == SerialParser::QUERY_IS_READY) {
         // Return 1 only if both motors have successfully found their home position
         if(isArmReady() && isJawReady()) {
             response += "1";
         } else {
             response += "0";
         }
      }
      
      // --- COMMAND: HAND HOME (STOP WAVING) ---
      // !HH
      else if (command == SerialParser::COMMAND_HAND_HOME) {
        noTone(kArmStepPin);
        armRunning = false;
      } 
      
      // --- COMMAND: HAND WAVE (START WAVING) ---
      // !HW
      else if (command == SerialParser::COMMAND_HAND_WAVE) {
        // We use the `tone` library for this as we care about speed but not
        // how far it moves.
        // tone() outputs a square wave at the specified frequency, either
        // until you stop it or until the optional time parameter elapses.
        tone(kArmStepPin, armSpeedMicrostepsPerSecond);
        armRunning = true;
      } 
      
      // --- COMMAND: MOUTH HOME (STOP TALKING) ---
      // !MH
      else if (command == SerialParser::COMMAND_MOUTH_HOME) {
      } 
      
      // --- COMMAND: MOUTH TALK (START TALKING) ---
      // !MW
      else if (command == SerialParser::COMMAND_MOUTH_TALK) {
      } 
      
      else {
        // Unknown command
        response += "?";
      }
      
      response += '\n';
      Serial.write(response.c_str(), response.length());
    }
  }
}

//here be ISR's

void jawHomeLimSwISR()
{
  jawLimitHit = true;
}
