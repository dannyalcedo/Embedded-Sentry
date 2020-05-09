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
unsigned char checkingInput(float accelVals[3][5]);

// Global Variables
float accelVals[3][5];
unsigned char currVal = 0; //current index for changing
unsigned char state = 0;
// 0 = x direction
// 1 = y direction
// 2 = z direction
unsigned char keyVals[3] = {1, 2, 0}; //move in x, then move in y, then move in z

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
    switch(state){
      case 0 :
        if (checkingInput(accelVals) == 3){
          Serial.print("Awaiting First Key Input");
          state = 0;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[0]){
          Serial.println();
          Serial.println("Your first input was correct! Now what's the second key value?");
          state = 1;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[0])){
          Serial.println();
          Serial.println("Your first input was not correct! Please start again...");
          state = 0;
          break;
        }
        break;
      case 1 :
        if (checkingInput(accelVals) == 3){
          Serial.print("Awaiting Second Key Input");
          state = 1;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[1]){
          Serial.println();
          Serial.println("Your second input was correct! Now what's the final key value?");
          state = 2;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[1])){
          Serial.println();
          Serial.println("Your second input was not correct! Please start again...");
          state = 0;
          break;
        }
        break;
      case 2 :
        //break
        if (checkingInput(accelVals) == 3){
          Serial.print("Awaiting Final Key Input");
          state = 2;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[2]){
          Serial.println();
          Serial.println("Your Final input was correct!");
          state = 3;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[2])){
          Serial.println();
          Serial.println("Your Final input was not correct! Please start again...");
          state = 0;
          break;
        }
        break;
      case 3 :
        digitalWrite(LED_BUILTIN, HIGH);   
        delay(100);                       
        digitalWrite(LED_BUILTIN, LOW);    
        delay(100);
        Serial.println("* * * You've correctly input the key! You should see the LED on the MC blink! * * *");
        Serial.println("                Reset the MC if you would like to try again");
    }
//    printRawAccel();
//    checkingInput(accelVals);
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

unsigned char checkingInput(float accelVals[3][5]){
  for (int i = 0; i < 5; i = i + 1){
    if (abs(imu.calcAccel(accelVals[0][i])*9.8066) > 4){
//      Serial.println(imu.calcAccel(accelVals[0][i])*9.8066);
      //Serial.println("You moved in the X direction");
      return 0;
    }
    else if (abs(imu.calcAccel(accelVals[1][i])*9.8066) > 3){
//      Serial.println(imu.calcAccel(accelVals[1][i])*9.8066);
      //Serial.println("You moved in the Y direction");
      return 1;
    }
    else if ((imu.calcAccel(accelVals[2][i])*9.8066) > 11 | (imu.calcAccel(accelVals[2][i])*9.8066) < 7){
//      Serial.println(imu.calcAccel(accelVals[2][i])*9.8066);
      //Serial.println("You moved in the Z direction");
      return 2;
    }
//    else {
//      Serial.println("Didn't move");
//    }
  }
  return 3;
}