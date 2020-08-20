#include <analogWrite.h> // ESP32 analogWrite implementation taken from https://github.com/ERROPiX/ESP32_AnalogWrite

// rough draft of a state machine to control the hand sanitization process.
// to do: connect to a
#include <HardwareSerial.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <FirebaseJson.h>
#include "time.h"

// WIFI SETUP
// MUST FILL OUT THE FOLLOWING
#define FIREBASE_HOST "XXXX"
#define FIREBASE_AUTH "XXXX"
#define WIFI_SSID "XXXX"
#define WIFI_PASSWORD "XXXX"

FirebaseData firebaseData;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -28800;
const int   daylightOffset_sec = 3600;

// OUTPUT PINS
int BRINE_EN = 17;
int BRINE1 = 18;
int BRINE2 = 19;
int GEN_PIN = 22;
int SOLENOID_PIN = 23;

// INPUT PINS
int BRINE_PIN = 32;

// CONTROL

int GEN_DC = 102;

boolean en = true;
boolean err = false;

int BRINE_TIME = 2; // time to run brine disp (sec)
unsigned long lastBrine; // tracker for last brine
unsigned long BRINE_DOWN = 5; // maximum time for brine to be down (sec)
boolean brineOn; // brine status

int SOLENOID_TIME = 2; // time to run solenoid (sec)
unsigned long lastSol; // tracker for last solenoid
boolean solenoidOn; // solenoid status

int GEN_TIME = 10; // time to run generator (sec)
unsigned long lastGen; // tracker for last generation
boolean genOn; // generator status

time_t currentTime; // variable for pseudo-threading


// FUNC DECLARATIONS
void getLocalTime();
void debugPrint();
void store_error_fb();

// FUNC DEFNS
void IRAM_ATTR brine_isr() { // enables brine dispensing
  if (!brineOn) {
    lastBrine = currentTime;
    digitalWrite(BRINE_EN, HIGH);
    brineOn = true;
  }
  Serial.print("brine button pressed\n");
  debugPrint();
}

void IRAM_ATTR solenoid_isr() { // enables solenoid dispensing
  if (!solenoidOn) {
    lastSol = currentTime;
    digitalWrite(SOLENOID_PIN, HIGH);
    solenoidOn = true;
  }
  Serial.print("solenoid button pressed\n");
  debugPrint();
}

void IRAM_ATTR gen_isr() { // enables solenoid dispensing
  if (!genOn) {
    lastGen = currentTime;
    analogWrite(GEN_PIN, GEN_DC);
    genOn = true;
  }
  Serial.print("gen button pressed\n");
  debugPrint();
}

void debugPrint() {
  Serial.print("current time: ");
  Serial.print(currentTime);
  Serial.print("\n");
  Serial.print("last sol time: ");
  Serial.print(lastSol);
  Serial.print("\n");
  Serial.print("last gen time: ");
  Serial.print(lastGen);
  Serial.print("\n\n");
}

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void store_error_fb() {
  FirebaseJson error_data;
  error_data.set("time", 1);
  error_data.set("reason", "filler");
  // ADD SENSOR DATA HERE

  if (Firebase.pushJSON(firebaseData, "/error_log", error_data)) {
    Serial.println("Firebase store successful.");
  } else {
    Serial.print("Error: ");
    Serial.println(firebaseData.errorReason());
  }
}

void streamCallback(StreamData data) {
  if (data.dataType() == "boolean") {
    if (data.boolData()) {
      Serial.println("Maintenance needed! Waiting...");
      en = false;
    } else {
      Serial.println("Resolved.");
      en = true;
    }
  }
}


void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Stream timeout, resume streaming...");
  }
}

void setup() {
  // SERIAL SETUP
  Serial.begin(57600);

  // WIFI SETUP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\nConnecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // FIREBASE SETUP
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(firebaseData, "interrupt"))
  {
    Serial.println("Could not begin interrupt stream");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }

  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);

  // TIME SETUP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime();

  // MOTOR SETUP
  pinMode(BRINE_EN, OUTPUT);
  pinMode(BRINE1, OUTPUT);
  pinMode(BRINE2, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(GEN_PIN, OUTPUT);

  pinMode(BRINE_PIN, INPUT_PULLUP);
  attachInterrupt(BRINE_PIN, gen_isr, FALLING);


  currentTime = 0;
  lastBrine = 0;
  lastGen = 0;
  lastSol = 0;
  brineOn = false;
  genOn = false;
  solenoidOn = false;
}

void loop() {
  digitalWrite(BRINE1, HIGH);
  digitalWrite(BRINE2, LOW);

  while (en) {
    time(&currentTime);
    if (brineOn) {
      if (currentTime - lastBrine > BRINE_TIME) {
        Serial.print("brine off \n");
        lastBrine = currentTime;
        brineOn = false;
        digitalWrite(BRINE_EN, LOW);
      }
    } else {
      if (currentTime - lastBrine > BRINE_DOWN) {
        Serial.print("brine on \n");
        lastBrine = currentTime;
        brineOn = true;
        digitalWrite(BRINE_EN, HIGH);
      }
    }

    if (solenoidOn) {
      if (currentTime - lastSol > SOLENOID_TIME) {
        Serial.print("sol off \n");
        solenoidOn = false;
        digitalWrite(SOLENOID_PIN, LOW);
      }
    }

    if (genOn) {
      if (currentTime - lastGen > GEN_TIME) {
        Serial.print("gen off \n");
        genOn = false;
        analogWrite(GEN_PIN, 0);
      }
    }

    //store_error_fb();
  }
}
