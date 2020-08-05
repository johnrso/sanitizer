// rough draft of a state machine to control the hand sanitization process.
// to do: connect to a 
#include <HardwareSerial.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"

// WIFI SETUP
// MUST FILL OUT THE FOLLOWING
#define FIREBASE_HOST "YOUR_FIREBASE_PROJECT.firebaseio.com"
#define FIREBASE_AUTH "YOUR_FIREBASE_DATABASE_SECRET"
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASS"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void debugPrint(); // debug print statement
FirebaseData firebaseData;

// MOTOR PINS

int BRINE_EN = 17;
int BRINE1 = 18;
int BRINE2 = 19;
int BRINE_PIN = 32;

// CONTROL

int BRINE_TIME = 2000; // time to run brine disp
unsigned long lastBrine; // tracker for last brine
unsigned long BRINE_DOWN = 5000; // maximum time for brine to be down
boolean brineOn;

unsigned long currentTime; // variable for pseudo-threading

void IRAM_ATTR brine_isr() {
  if (!brineOn) {
    lastBrine = currentTime;
    digitalWrite(BRINE_EN, HIGH);
    brineOn = true;
  }
  Serial.print("brine button pressed\n");
  debugPrint();
}

void setup() {
  // SERIAL SETUP
  Serial.begin(57600);

  // WIFI SETUP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // MOTOR SETUP
  pinMode(BRINE_EN, OUTPUT);
  pinMode(BRINE1, OUTPUT);
  pinMode(BRINE2, OUTPUT);
  
  pinMode(BRINE_PIN, INPUT_PULLUP);
  attachInterrupt(BRINE_PIN, brine_isr, FALLING);

  
  currentTime = 0;
  lastBrine = 0;
  brineOn = false;
}

void loop() {
  delay(50); // ensure that current time is not 0
  digitalWrite(BRINE1, HIGH);
  digitalWrite(BRINE2, LOW);
  
  while(currentTime = millis()) {
    if (brineOn) {
      if (currentTime - lastBrine > BRINE_TIME) {
        lastBrine = currentTime;
        brineOn = false;
        digitalWrite(BRINE_EN, LOW);  
      }
    } else {
      if (currentTime - lastBrine > BRINE_DOWN) {
        lastBrine = currentTime;
        brineOn = true;
        digitalWrite(BRINE_EN, HIGH);
      }
    }
  }

  if (currentTime % 1000 < 100) {
    debugPrint();
  }
}

/* debug print statement */
void debugPrint() {
  Serial.print("current time: ");
  Serial.print(currentTime);
  Serial.print("\n\n");
}
