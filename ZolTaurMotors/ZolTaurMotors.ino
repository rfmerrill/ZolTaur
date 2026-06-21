/*
 * Pawprint Prototyping:
 * 
 * Arduino Mega
 * Libraries Used:
 */

#include "SerialController.h"
#include <string.h>


//Debug pin
uint8_t debugPin = 21;


//Motors ///////////////////////

//Arm speed in tenths of a degree per second
uint16_t armSpeed = 300;
//Arm Per phase current
//uint16_t armCurrent = 500;
//Arm limit in deci degrees. 300 == 30 degrees
uint16_t armLimitDeciDeg = 200;

//Jaw Speed
uint16_t jawSpeed = 50;
//uint16_t jawCurrent = 500;
//Jaw home angle is how far from home the position target is set to. 
//The goal is to get close to home without setting off the limit switch
uint16_t jawHomeAngle = 25;
uint16_t jawLimitDeciDeg = 200;

// Serial Parser
SerialParser serialParser;
String outputBuffer;


//Input Pin numbers
uint8_t  EmergencyStopBttnPin = 8;
//uint8_t  ArmHmLimSwPin = 20;
//uint8_t  ArmOpenLimSwPin = 19;
uint8_t  JawHmLimSwPin = 18;


//uint8_t ledPin = LED_BUILTIN;
//volatile bool ledState = true;

volatile bool jawLimitActive = false;
volatile bool armLimitActive = false;

void setup()
{
  // put your setup code here, to run once:
  
  //Set Debug Pin
  pinMode(debugPin, INPUT);

  //Enable Serial and wait for it to connect.
  Serial.begin(9600);
  while (!Serial) {}

  
  //What is this pin for?
  pinMode(53, OUTPUT);

  //attach interrupts
  //attachInterrupt(digitalPinToInterrupt(EmergencyStopBttn.pinNumber), emergencyStopBttnISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(ArmController.HomeLimitSwitch.pinNumber), armOpenLimSwISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(ArmHmLimitSwitch.pinNumber), armOpenLimSwISR, RISING);
  attachInterrupt(digitalPinToInterrupt(1), jawHomeLimSwISR, RISING);
}

bool isArmHome() {
  return true;
}

bool isJawHome() {
  return true;
}

// put your main code here, to run repeatedly:
void loop()
{
  // motor updates go here
  updateSerial();
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
         if(isArmHome() && isJawHome()) {
             response += "1";
         } else {
             response += "0";
         }
      }
      
      // --- COMMAND: HAND HOME (STOP WAVING) ---
      // !HH
      else if (command == SerialParser::COMMAND_HAND_HOME) {
      } 
      
      // --- COMMAND: HAND WAVE (START WAVING) ---
      // !HW
      else if (command == SerialParser::COMMAND_HAND_WAVE) {
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
void armOpenLimSwISR()
{
  armLimitActive = true;
}

void jawHomeLimSwISR()
{
  jawLimitActive = true;
}
