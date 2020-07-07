// rough draft of a state machine to control the hand sanitization process.

#include <HardwareSerial.h>

int NUM_SOLENOIDS = 3; // number of solenoid pins, make sure this = length of SOLENOIDS 
int NUM_DISP = 3; // number of dispensers, make sure this = length of DISPS

int SOLENOIDS[] = {2, 3, 4}; // array of solenoid pins
int *solenoidStates; // array of solenoid states, 1 = closed, 0 = open

int PUMP; // pump motor
int DISPS[] = {5, 6, 7}; // array of water level sensors
int *levelReadings; // array of sensor readings

unsigned long GENERATION_TIME = 1000; // time for HClO generation to run 
unsigned long GENERATION_DOWN_TIME = 2000; // time in between HClO generation

int ACC_LEVEL_THRESHOLD; // acceptable level of water to fill 
int UNACC_LEVEL_THRESHOLD; //level of water to signal a refill

int GENERATOR; // pin for controlling the generation of HCLO

int genStatus; // boolean state of the generator
unsigned long lastGeneration; // variable for tracking the last time when the generator was powered.
unsigned long genOn; // variable for the time at which the generator was powered on.

unsigned long currentTime; // variable for pseudo-threading

void debugPrint(); // debug print statement

void setup() {
  Serial.begin(57600); // communication through serial port
  
  solenoidStates = (int *)calloc(sizeof(int), NUM_SOLENOIDS);
  levelReadings = (int *)calloc(sizeof(float), NUM_DISP);
  
  for (int i = 0; i < NUM_SOLENOIDS; i++) {
    pinMode(SOLENOIDS[i], OUTPUT);
  }

  for (int i = 0; i < NUM_DISP; i++) {
    pinMode(DISPS[i], INPUT);
  }

  pinMode(GENERATOR, OUTPUT);
  lastGeneration = 0; // look into making last generation 
  genStatus = 0;
  genOn = 0;
}

void loop() {
  delay(50); // ensure that current time is not 0
  
  while(currentTime = millis()) {
  
    if (genStatus) { // if generator on
      if ((currentTime - genOn) >= GENERATION_TIME) {
        lastGeneration = currentTime;
        digitalWrite(GENERATOR, LOW);
        genStatus = 1 - genStatus;
        debugPrint();
      }
    } else {
      if ((currentTime - lastGeneration) >= GENERATION_DOWN_TIME) {
        genOn = currentTime;
        digitalWrite(GENERATOR, HIGH);
        genStatus = 1 - genStatus;
        debugPrint();
      }
    }

    for (int i = 0; i < NUM_SOLENOIDS; i++) {
      levelReadings[i] = analogRead(DISPS[i]);
      if (!solenoidStates[i] && levelReadings[i] > ACC_LEVEL_THRESHOLD) {
        solenoidStates[i] = 1;
      } else if (solenoidStates[i] && levelReadings[i] < UNACC_LEVEL_THRESHOLD) {
        solenoidStates[i] = 0;
      }
    }
  }
}

/* debug print statement */
void debugPrint() {
  Serial.print("current time: ");
  Serial.print(currentTime);
  Serial.print(" ms\ngenerator status: ");
  Serial.print(genStatus ? "on" : "off");
  Serial.print("\nsensor readings: \n");
  for (int i = 0; i < NUM_DISP; i++) {
    Serial.print("    sensor");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(levelReadings[i]);
    Serial.print("\n");
  }
  Serial.print("\n");
}
