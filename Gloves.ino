#include "SD.h"
#define SD_ChipSelectPin 4
#include "TMRpcm.h"
#include "SPI.h"


TMRpcm tmrpcm;

// Define the sensor values for each sign
int signValues[][5] = {
  {45, 72, 67, 63, 45}, // A
  {45, 0, 0, 60, 0},  // i want to eat
  {45, 2, 80, 67, 47},  // ب
  {45, 50, 48, 0, 0},  // ahln
  {45, 0, 0, 61, 50},  // ت
  {45, 0, 80, 70, 5},  // i love you
};
int onVlaues[4] = {68, 66, 66, 46}; // for index, middle, ring & pinky,.. if more than these values, glove is considered closed
int offVlaues[4] = {4,4,4,4}; // for index, middle, ring & pinky,.. if less than these values, glove is considered extended
const char signs[7] = { // Letter value
  'A', // Thumb and Index fingers extended, other fingers closed
  '1', // i want to eat
  '2', // ب
  '3', // ahln
  '4', // ت
  '5', // i love you
};
// Define the pins for each flex sensor
const int thumbPin = A4;
const int indexPin = A3;
const int middlePin = A2;
const int ringPin = A1;
const int pinkyPin = A0;
const int redLedPin = 2;
const int greenLedPin = 3;
const int blueLedPin = 5;

int thumbMin = 1023;
int thumbMax = 0;
int indexMin = 1023;
int indexMax = 0;
int middleMin = 1023;
int middleMax = 0;
int ringMin = 1023;
int ringMax = 0;
int pinkyMin = 1023;
int pinkyMax = 0;
//
const int offsetError = 7; // +-7
// Speaker
const int speakerPin = 9;
//
int geastureStatus = 0; // check for on/off geasture
int detectedCount = 0; // between on/off moves
bool glovesOn = false;


bool compare(long int v1, long int v2, long int addToOffset=0) {
  if(v1==v2 || (v2 > v1-(offsetError+addToOffset) && v2 < v1+(offsetError+addToOffset)))
    return true;
  return false;
}
int getGesture(long int thumb, long int index, long int middle, long int ring, long int pinky) {
  int detectedSign = -1;
  int len = sizeof(signValues) / sizeof(signValues[0]);
  for (int i = 0; i < len; i++) {
    if(compare(signValues[i][0], thumb, 45) && 
       compare(signValues[i][1], index, 4) &&
       compare(signValues[i][2], middle, 4) && 
       compare(signValues[i][3], ring, 4) && 
       compare(signValues[i][4], pinky, 4)) {
      detectedSign = i;
      break;
    }
  }
  return detectedSign;
}
int checkOnOff(long int index, long int middle, long int ring, long int pinky) {
  if(index >= onVlaues[0] && middle >= onVlaues[1] && ring >= onVlaues[2] && pinky >= onVlaues[3])  {
    return 1;
  } 
  if(index <= offVlaues[0] && middle <= offVlaues[1] && ring <= offVlaues[2] && pinky <= offVlaues[3])  {
    return 0;
  } 
  return -1;
}
void setup() {
  Serial.begin(9600);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  // SD Card connection
  tmrpcm.speakerPin = speakerPin;
  if (!SD.begin(SD_ChipSelectPin)) {
  Serial.println("SD fail");
  return;
  }
  else {
    Serial.println("SD Connected");
  }

  tmrpcm.setVolume(6);
  // Callibrating
  while(millis()<8000)
  {
    if(!glovesOn) {
      digitalWrite(blueLedPin, HIGH);
      digitalWrite(greenLedPin, LOW);
      digitalWrite(redLedPin, LOW);
      int flexADC1 = analogRead(thumbPin);
      int flexADC2 = analogRead(indexPin);
      int flexADC3 = analogRead(middlePin);
      int flexADC4 = analogRead(ringPin);
      int flexADC5 = analogRead(pinkyPin);

      if(flexADC1<thumbMin)
      {
        thumbMin=flexADC1;
      }
      if(flexADC1>thumbMax)
      {
        thumbMax=flexADC1;
      }

      if(flexADC2<indexMin)
      {
        indexMin=flexADC2;
      }
      if(flexADC2>indexMax)
      {
        indexMax=flexADC2;
      }
      if(flexADC3<middleMin)
      {
        middleMin=flexADC3;
      }
      if(flexADC3>middleMax)
      {
        middleMax=flexADC4;
      }
      if(flexADC4<ringMin)
      {
        ringMin=flexADC4;
      }
      if(flexADC4>ringMax)
      {
        ringMax=flexADC4;
      }
      if(flexADC5<pinkyMin)
      {
        pinkyMin=flexADC5;
      }
      if(flexADC5>pinkyMax)
      {
        pinkyMax=flexADC5;
      }

    }
  }
  glovesOn = false;
}

void loop() {
  // Read the values from each flex sensor
  if(glovesOn) {
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
        digitalWrite(blueLedPin, LOW);
  }
  else {
        digitalWrite(redLedPin, HIGH);
        digitalWrite(greenLedPin, LOW);
        digitalWrite(blueLedPin, LOW);
  }
  int thumbValue = analogRead(thumbPin);
  int indexValue = analogRead(indexPin);
  int middleValue = analogRead(middlePin);
  int ringValue = analogRead(ringPin);
  int pinkyValue = analogRead(pinkyPin);
  thumbValue = constrain(thumbValue,thumbMin, thumbMax);
  indexValue = constrain(indexValue,indexMin, indexMax);
  middleValue = constrain(middleValue,middleMin, middleMax);
  ringValue = constrain(ringValue,ringMin, ringMax);
  pinkyValue = constrain(pinkyValue,pinkyMin, pinkyMax);


  thumbValue = map(thumbValue, thumbMin, thumbMax, 0, 90);
  indexValue = map(indexValue, indexMin, indexMax, 0, 90);
  middleValue = map(middleValue, middleMin, middleMax, 0, 90);
  ringValue =  map(ringValue, ringMin, ringMax, 0, 90);
  pinkyValue = map(pinkyValue, pinkyMin, pinkyMax, 0, 90); 
  //// Determine which sign is being made
  char sign;
  // Determine which sign is being made based on the sensor values
  int detectedSign = getGesture(thumbValue, indexValue, middleValue, ringValue, pinkyValue);
   // for debugging purposes
   Serial.print("\nstep: ");
   Serial.print(geastureStatus);
   Serial.print("\ndetectedSign: ");
   Serial.print(detectedSign);
   if(detectedSign != -1) {
          sign = signs[detectedSign];
          Serial.print(sign);
   }
     
   Serial.print("\nThumb: ");
   Serial.print(thumbValue);
   Serial.print(", ");
   Serial.print(" Index: ");
   Serial.print(indexValue);
   Serial.print(", ");
   Serial.print(" Middle: ");
   Serial.print(middleValue);
   Serial.print(", ");
   Serial.print(" Ring: ");
   Serial.print(ringValue);
   Serial.print(", ");
   Serial.print(" Pinky: ");
   Serial.print(pinkyValue);

   Serial.print("\nThumb V: ");
   Serial.print(thumbMin);
   Serial.print(", ");
   Serial.print(thumbMax);
   Serial.print(", ");
   Serial.print(" Index V: ");
   Serial.print(indexMin);
   Serial.print(", ");
   Serial.print(indexMax);
   Serial.print(", ");
   Serial.print(" Middle: ");
   Serial.print(middleMin);
   Serial.print(", ");
   Serial.print(middleMax);
   Serial.print(", ");
   Serial.print(" Ring: ");
   Serial.print(ringMin);
   Serial.print(", ");
   Serial.print(ringMax);
   Serial.print(", ");
   Serial.print(" Pinky: ");
   Serial.print(pinkyMin);
   Serial.print(", ");
   Serial.print(pinkyMax);
 // check if doing on/off geastures
 if (detectedSign == -1) {
     int onOff = checkOnOff(indexValue, middleValue, ringValue, pinkyValue);
     if(onOff == 1) {
       Serial.print("\nclosed");
       if(geastureStatus == 0 || geastureStatus == 2) {
         geastureStatus += 1;
         detectedCount = 0;
       }
     }
     else if(onOff == 0) {
       Serial.print("\nextended");
       if(geastureStatus == 1) {
         geastureStatus = 2;
         detectedCount = 0;
       }
       if(geastureStatus == 3) {
         glovesOn = !glovesOn;
         geastureStatus = 0;
         detectedCount = 0;
       }
     }
     else {
       if (detectedCount > 4) {
         geastureStatus = 0;
         detectedCount = 0;
       }
     }
   }
 // Print & Play the detected sign, if any
 if(glovesOn) {
   if (detectedSign != -1) {
     Serial.print("\nDETECTED: ");
     Serial.print(sign);
     Serial.print("\n");
     const char sname[] = {sign, '.', 'w', 'a', 'v', '\0'};
     Serial.println(sname);
     if(tmrpcm.wavInfo(sname) == 1) {
       tmrpcm.play(sname);
       delay(2000); 
     }
     detectedCount += 1;
   }

 }
  delay(500);
}