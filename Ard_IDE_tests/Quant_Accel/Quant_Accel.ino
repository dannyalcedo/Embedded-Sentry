#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include <stdlib.h>

LSM9DS1 imu;

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

#define DECLINATION -13.68 // Declination (degrees) in Shelton, CT.

//Function definitions
void printRawAccel();
void checkingInput(float accelVals[3][5]);

// Global Variables
float accelVals[3][5];
unsigned char currVal = 0; //current index for changing

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (imu.begin() == false) {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    while (1);
  }
}

void loop() {
  if ( imu.accelAvailable() )
  {
    imu.readAccel();
    accelVals[0][currVal] =  imu.ax;
    accelVals[1][currVal] =  imu.ay;
    accelVals[2][currVal] =  imu.az;
  }

  if (currVal == 4){currVal = 0;}
  else {currVal = currVal + 1;}
  
  if ((lastPrint + PRINT_SPEED) < millis())
  {
//    printRawAccel();
    checkingInput(accelVals);
    Serial.println();

    lastPrint = millis(); // Update lastPrint time
  }
}

void printRawAccel() {
  Serial.print("A: ");
  float accelXms2 = imu.calcAccel(imu.ax) * 9.8066;
  float accelYms2 = imu.calcAccel(imu.ay) * 9.8066;
  float accelZms2 = imu.calcAccel(imu.az) * 9.8066;
  Serial.print(accelXms2, 2);
  Serial.print(", ");
  Serial.print(accelYms2, 2);
  Serial.print(", ");
  Serial.print(accelZms2, 2);
}

void checkingInput(float accelVals[3][5]){
  for (int i = 0; i < 5; i = i + 1){
    if (abs(imu.calcAccel(accelVals[0][i])*9.8066) > 4){
      Serial.println("You moved in the X direction");
    }
    else if (abs(imu.calcAccel(accelVals[1][i])*9.8066) > 3){
      Serial.println("You moved in the Y direction");
    }
    else if ((imu.calcAccel(accelVals[2][i])*9.8066) > 11 | (imu.calcAccel(accelVals[2][i])*9.8066) < 7){
      Serial.println("You moved in the Z direction");
    }
    else {
      Serial.println("You didn't move");
    }
  }
}
