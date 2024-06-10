#include <NewPing.h>
#include <RunningMedian.h>
#include "AVR_PWM.h"

// trigger and echo pins for each sensor
#define SONAR1 7 
#define SONAR2 8
#define SONAR3 2
#define SONAR4 9
#define SONAR5 4
#define SONAR6 5

#define NUM_SONAR 6 // number of sonar sensors

#define neon 3 // digital pin to control neon

#define MAX_DISTANCE 200 // maximum distance for sensors
#define MIN_DISTANCE 0

NewPing sonar[NUM_SONAR] = { // array of sonar sensor objects
  NewPing(SONAR1, SONAR1, MAX_DISTANCE),
  NewPing(SONAR2, SONAR2, MAX_DISTANCE),
  NewPing(SONAR3, SONAR3, MAX_DISTANCE),
  NewPing(SONAR4, SONAR4, MAX_DISTANCE),
  NewPing(SONAR5, SONAR5, MAX_DISTANCE),
  NewPing(SONAR6, SONAR6, MAX_DISTANCE)
};

// setup a median object
RunningMedian samples = RunningMedian(5);

int distance[NUM_SONAR]; // array stores distances for each sensor in cm
int medianVal; // stores median value
int minVal; // stores the smallest value
int brightness; // stores brightness converted from min distance reading
// neon max brightenss is 0 and min is 255
int real_brightness; // contrain brightness

void setup() {
  Serial.begin(115200);
  pinMode(neon, OUTPUT);
  //PWM setup; use Timer 2 (Pin D3)  
  TCCR2A = bit (WGM20) | bit (WGM21) | bit (COM2B1); // fast PWM, clear OC2B on compare
  TCCR2B = bit (WGM22) | bit (CS20) | bit (CS21) | bit (CS22); // prescaler of 1024
  OCR2A = 125;    // zero relative  132 ~ 135?
  OCR2B = 125;    // 100% duty cycle
}

void loop() {
  
  updateSonar(); // update the distance array
  updateNeon();
  analogWrite(neon, real_brightness);

  //print all distances
  // Serial.print("  Sonar 1: ");
  // Serial.print(distance[0]);
  // Serial.print("  Sonar 2: ");
  // Serial.print(distance[1]);
  // Serial.print("  Sonar 3: ");
  // Serial.print(distance[2]);
  // Serial.print("  Sonar 4: ");
  // Serial.print(distance[3]);
  // Serial.print("  Sonar 5: ");
  // Serial.print(distance[4]);
  // Serial.print("  Sonar 6: ");
  // Serial.print(distance[5]);

  // Serial.print("  minVal: ");
  // Serial.print(minVal);
  // Serial.print("  Median: ");
  // Serial.print(medianVal);

  // Serial.print("  Brightness: ");
  // Serial.print(brightness);
  // Serial.print("  Real Brightness: ");

  // Send brightness value to MAX
  Serial.println(real_brightness);
}

void updateSonar() {
  for (int i = 0; i < NUM_SONAR; i++) {
    distance[i] = sonar[i].ping_cm(); // update distance from each sensor
  
    if (distance[i] == 0){
    // if nothing is detected, set value to max distance
      distance[i] = MAX_DISTANCE;
    }

    // Out of range handleling
    if (distance[i] < MIN_DISTANCE){
      distance[i] = 0;
    }
    if (distance[i] > MAX_DISTANCE){
      distance[i] = MAX_DISTANCE;
    }
  }

  // return the distance of the closest object
  minVal = min(distance[0],
           min(distance[1], 
           min(distance[2], 
           min(distance[3], 
           min(distance[4], distance[5])))));
}

void updateNeon() {
  // get median value from the last 5 readings to filter out outliers
  samples.add(minVal);
  medianVal = samples.getMedian();
  // map the filtered value to neon's brightness range
  brightness = map(medianVal, 0, MAX_DISTANCE, 0, 253);
  // constrain the brightness value
  real_brightness = constrain(brightness, 0, 253);

  //write brightness value to the neon
  //analogWrite(neon, real_brightness);
}