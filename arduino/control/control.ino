// rough draft of a state machine to control the hand sanitization process.

int NUM_SOLENOIDS; // number of solenoid pins
int NUM_DISP; // number of dispensers

int SOLENOIDS[NUM_SOLENOIDS]; // array of solenoid pins
int solenoidStates[NUM_SOLENOIDS]; // array of solenoid states, 1 = closed, 0 = open

int PUMP; // pump motor
int DISPS[NUM_DISP]; // array of water level sensors
float levelReadings[NUM_DISP]; // array of sensor readings

unsigned long GENERATION_TIME; // time for HClO generation to run 
unsigned long GENERATION_DOWN_TIME; // time in between HClO generation

float ACC_LEVEL_THRESHOLD; // acceptable level of water to fill 
float UNACC_LEVEL_THRESHOLD; //level of water to signal a refill

int GENERATOR; // pin for controlling the generation of HCLO

int genStatus; // boolean state of the generator
unsigned long lastGeneration; // variable for tracking the last time when the generator was powered.
unsigned long genOn; // variable for the time at which the generator was powered on.

void setup() {
  Serial.begin(57600); // communication through serial port
  
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
  
  unsigned long currentTime;
  delay(50); // ensure that current time is not 0
  
  while(currentTime = millis()) {
  
    if (genStatus) { // if generator on
      if ((currentTime - genOn) >= GENERATION_TIME) { // if generator has been on for long enough
        lastGeneration = curentTime;
        digitalWrite(GENERATOR, LOW);
        genStatus = 1 - genStatus;
      }
    } else {
      if ((currentTime - lastGeneration) >= GENERATION_DOWN_TIME) { // if generator has been off long enough
        genOn = curentTime;
        digitalWrite(GENERATOR, HIGH);
        genStatus = 1 - genStatus;
      }
    }

    for (int i = 0; i < NUM_DISP; i++) {
      DISPS[i] = analogRead(
    }
    
  }
}
