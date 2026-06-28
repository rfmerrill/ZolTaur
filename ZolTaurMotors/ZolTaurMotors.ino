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
constexpr int jawSpeedDeciDegPerSec = 250;
constexpr int jawMoveDeciDeg = 900; // Movement arc for each normal back-and-forth
constexpr int jawExtraLimitMoveDeciDeg = 150; // Extra movement away from limit to avoid hitting it again
constexpr int jawStepsPerRevolution = 200;
constexpr int jawMicrostepsPerStep = 8;
constexpr int kJawLimitWaitTimeMs = 300;

// BasicStepperDriver library takes float RPM for speed. We match that.
constexpr float jawSpeedRevsPerMin = (jawSpeedDeciDegPerSec * 60.0) / 3600.0;
static_assert(jawSpeedRevsPerMin == 4.166666666666667);

//Jaw home angle is how far from home the position target is set to. 
//The goal is to get close to home without setting off the limit switch
constexpr int jawHomeAngle = 25;
// Set this to either 1 or -1
constexpr int jawDirectionAwayFromLimit = 1;
constexpr int jawDirectionTowardsLimit = -jawDirectionAwayFromLimit;

constexpr uint8_t kArmDirPin = 8;
// Pin 9 is OC2A so we could use timer compare out
// if the tone() library isn't good enough.
constexpr uint8_t kArmStepPin = 9;
constexpr uint8_t kArmSleepPin = 10;

constexpr uint8_t kJawDirPin = 11;
constexpr uint8_t kJawStepPin = 12;
constexpr uint8_t kJawSleepPin = 13;
// Limit switch should be normally open to ground, and connected to
// a pin that Arduino supports interrupt on (2, 3, 18-21 on Mega?)
constexpr uint8_t kJawLimitSwitchPin = 3;

// Serial Parser
SerialParser serialParser;
String outputBuffer;

// Stepper driver for jaw
BasicStepperDriver jawStepper(jawStepsPerRevolution, kJawDirPin, kJawStepPin, kJawSleepPin);

// Did we see a falling edge on the limit switch?
volatile bool jawLimitHit = false;
bool jawRunning = false;
bool jawRequested = false;
int jawDirection = jawDirectionTowardsLimit;
bool jawDoRestartMove = false;

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
  attachInterrupt(digitalPinToInterrupt(kJawLimitSwitchPin), jawHomeLimSwISR, FALLING);

  jawStepper.begin(jawSpeedRevsPerMin, jawMicrostepsPerStep);

  initJawStepper();
}

// put your main code here, to run repeatedly:
void loop()
{
  // motor updates go here
  updateJawStepper();
  updateSerial();
}

inline bool isLimitSwitchClosed() {
  return digitalRead(kJawLimitSwitchPin) == LOW;
}

void initJawStepper() {
  jawDirection = jawDirectionTowardsLimit;
  jawRunning = true;

  if(isLimitSwitchClosed()) {
    // we're already sitting on the limit switch
    jawLimitHit = true;
  } else {
    // Intentionally hit limit to home
    long movement_deci_deg = jawMoveDeciDeg * 2;
    long steps = jawStepper.calcStepsForRotation(jawDirection * movement_deci_deg) / 10;
    jawStepper.startMove(steps);
  }
}

void updateJawStepper() {
  unsigned long wait_until = 0;
  static long movement_deci_deg = jawMoveDeciDeg;

  if (!jawRunning) {
    if (!jawRequested) {
      return;
    } else {
      // Start of requested move. Assume we are home.
      jawDirection = jawDirectionAwayFromLimit;
      jawDoRestartMove = true;

      // Normally we care about limit switch edges.
      // however, if we were not running, then this value
      // may be stale.
      jawLimitHit = isLimitSwitchClosed();
    }
  }

  jawRunning = true;

  if (jawLimitHit) {
    // TODO: which direction is away from the limit switch?
    jawDirection = jawDirectionAwayFromLimit;
    wait_until = millis() + kJawLimitWaitTimeMs;
    // This will probably get activated more than once due to bounce, just
    // need to make sure we don't start moving until it settles.
    jawLimitHit = false;
    movement_deci_deg += jawExtraLimitMoveDeciDeg;
    jawDoRestartMove = true;
  } else if (wait_until > millis()) {
    // pass
  } else if (jawDoRestartMove) {
    jawDoRestartMove = false;
    // Rather than use the float version of this function, pass it
    // decidegrees and then divide by 10 after.
    // Also, don't actually multiply by direction--compiler is too dumb to
    // know it's always 1 or -1.
    long steps;
    if (jawDirection > 0) {
      steps = jawStepper.calcStepsForRotation(movement_deci_deg) / 10;
    } else {
      steps = jawStepper.calcStepsForRotation(-movement_deci_deg) / 10;
    }
    jawStepper.startMove(steps);
  } else {
    // run the next action and see if the commanded movement is done
    bool moveDone = (jawStepper.nextAction() == 0);
    if (moveDone) {
      if (!jawRequested && jawDirection == jawDirectionTowardsLimit) {
        // we were moving towards limit and ended without hitting it.
        // We are home!
        jawRunning = false;
      } else {
        // switch directions
        jawDirection = -jawDirection;
        // start a new move
        jawDoRestartMove = true;
        movement_deci_deg = jawMoveDeciDeg;
      }
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
        jawRequested = false;
      } 
      
      // --- COMMAND: MOUTH TALK (START TALKING) ---
      // !MW
      else if (command == SerialParser::COMMAND_MOUTH_TALK) {
        jawRequested = true;
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
