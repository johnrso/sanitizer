// rough draft of a state machine to control the hand sanitization process.

int NUM_SOLENOIDS; // number of solenoid pins
int NUM_DISP; // number of dispensers

int SOLENOIDS[NUM_SOLENOIDS]; // array of solenoid pins
int solenoidStates[NUM_SOLENOIDS]; // array of solenoid states, 1 = closed, 0 = open

int DISPS[NUM_DISP]; // array of water level sensors
float solenoidStatus[NUM_SOLENOIDS]; // array of sensor readings

unsigned long int GENERATION_TIME; // 

int GENERATOR; // pin for controlling the generation of HCLO

unsigned long lastGeneration; // variable for tracking the last time when the generator was powered.

void setup() {
  Serial.begin(57600); // communication through serial port
  
  for (int i = 0; i < NUM_SOLENOIDS; i++) {
    pinMode(SOLENOIDS[i], OUTPUT);
  }

  for (int i = 0; i < NUM_DISP; i++) {
    pinMode(DISPS[i], INPUT);
  }

  pinMode(GENERATOR, OUTPUT);
  lastGeneration = 0;
}

void loop() {
  while(1) {
    unsigned long currentTime = millis();
    
  }
}
