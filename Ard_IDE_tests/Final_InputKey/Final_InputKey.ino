// In this script, the key is input by the user but it is not consistent 
// the key is hard-coded into having only 3 values

#include <Wire.h> // default need for IMU
#include <SPI.h> // default need for IMU
#include <SparkFunLSM9DS1.h> // default need for IMU
#include <stdlib.h> // for abs() function in checkingInput()

LSM9DS1 imu; // initializing IMU object

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

//Function Definitions
void printRawAccel(); // debug function to see accelerometer values
unsigned char checkingInput(float accelVals[3][5]); // will return unsigned char mapped to a specifc movement
                                                    // 0 = x-axis, 1 = y-axis, 2 = z-axis, 3 = no movement
void blinkGreen();
void blinkRed();
void printKey(); // print key for debugging

// Global Variables
float accelVals[3][5]; // 2D vector acting as a window of sensor values in the x, y, and z axis' 
unsigned char currVal = 0; // current index for iterating through accelVals
unsigned char state = 0; // keep track of current state
unsigned char keyVals[3]; // array to hold key values from input
int LEDRIGHT = 3;
int LEDWRONG = 4;

void setup() {
  Serial.begin(115200); // setting baud rate
  pinMode(LEDRIGHT, OUTPUT);
  pinMode(LEDWRONG, OUTPUT);
  Wire.begin();
  if (imu.begin() == false) { // if connection fails
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    while (1);
  }
}

void loop() {
  if ( imu.accelAvailable() ) // reads accel data from sensor allowing imu.ax/ay/az to be set and populates accelVals array
  {
    imu.readAccel();
    accelVals[0][currVal] =  imu.ax;
    accelVals[1][currVal] =  imu.ay;
    accelVals[2][currVal] =  imu.az;
  }

  if (currVal == 4){currVal = 0;} // incrementing index to advance window
  else {currVal = currVal + 1;}
  
  if ((lastPrint + PRINT_SPEED) < millis())
  {
    switch(state){
      case 0 :
        Serial.print("Hello! You have yet to set a key. Please complete the following prompts to do so:");
        printKey();
        state = 1;
        break;
      case 1 :
        if (checkingInput(accelVals) == 3){ // no movement detected
          Serial.println("Awaiting First Key Input");
          state = 1;
          break;
        }
        else { // setting first key value
          keyVals[0] = checkingInput(accelVals);
          Serial.println("Your first key value has been accepted");
          blinkGreen();
          state = 2;
          break;
        }
      case 2 :
        if (checkingInput(accelVals) == 3){ // no movement detected
          Serial.println("Awaiting Second Key Input");
          state = 2;
          break;
        }
        else { // setting second key value
          keyVals[1] = checkingInput(accelVals);
          Serial.println("Your second key value has been accepted");
          blinkGreen();
          state = 3;
          break;
        }
      case 3 :
        if (checkingInput(accelVals) == 3){ // no movement detected
          Serial.println("Awaiting Final Key Input");
          state = 3;
          break;
        }
        else { // setting final key value
          keyVals[2] = checkingInput(accelVals);
          Serial.println("Your Final key value has been accepted");
          blinkGreen();
          Serial.println("You have successfuly input a 3-value key. Now you will be prompted to insert your key to unlock");
          printKey();
          state = 4;
          break;
        }
      case 4 : // checking for first movement to match first key value
        if (checkingInput(accelVals) == 3){ // no movement detected
          Serial.print("Awaiting First Key Value");
          state = 4;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[0]){ // if input from sensor is the first value in key
          Serial.println();
          Serial.println("Your first input was correct! Now what's the second key value?");
          blinkGreen();
          state = 5;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[0])){ // if first input from sensor is NOT first value in key
          Serial.println();
          Serial.println("Your first input was not correct! Please start again...");
          blinkRed(); 
          state = 4;
          break;
        }
        break;
      case 5 : // checking for second movement to match second key value
        if (checkingInput(accelVals) == 3){
          Serial.print("Awaiting Second Key Input");
          state = 5;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[1]){
          Serial.println();
          Serial.println("Your second input was correct! Now what's the final key value?");
          blinkGreen();
          state = 6;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[1])){
          Serial.println();
          Serial.println("Your second input was not correct! Please start again...");
          blinkRed(); 
          state = 4;
          break;
        }
        break;
      case 6 : // checking for final movement to match final key value
        if (checkingInput(accelVals) == 3){ 
          Serial.print("Awaiting Final Key Input");
          state = 6;
          break;
        }
        else if (checkingInput(accelVals) == keyVals[2]){
          Serial.println();
          Serial.println("Your Final input was correct!");
          blinkGreen();
          state = 7;
          break;
        }
        else if ((checkingInput(accelVals) != keyVals[2])){
          Serial.println();
          Serial.println("Your Final input was not correct! Please start again...");
          blinkRed(); 
          state = 4;
          break;
        }
        break;
      case 7 : // key was input correctly
        // blinking LED to show we've correctly input the key
        blinkGreen();
        Serial.println("* * * You've correctly input the key! You should see the LED on the MC blink! * * *");
        Serial.println("                Reset the MC if you would like to try again");
    }
//    printRawAccel(); // for debugging perposes
    Serial.println();

    lastPrint = millis(); // Update lastPrint time
  }
}

void printRawAccel() {
  Serial.print("A: ");
  // calculated values from sensor are in g's. Multiplying by 9.8066 to get unit in m/s^2
  float accelXms2 = imu.calcAccel(imu.ax) * 9.8066; 
  float accelYms2 = imu.calcAccel(imu.ay) * 9.8066;
  float accelZms2 = imu.calcAccel(imu.az) * 9.8066;
  Serial.print(accelXms2, 2);
  Serial.print(", ");
  Serial.print(accelYms2, 2);
  Serial.print(", ");
  Serial.print(accelZms2, 2);
}

unsigned char checkingInput(float accelVals[3][5]) {
  for (int i = 0; i < 5; i = i + 1){
    if (abs(imu.calcAccel(accelVals[0][i])*9.8066) > 4){ // hard coded value for quantization of movement in x-axis
      return 0; // moved in x direction
    }
    else if (abs(imu.calcAccel(accelVals[1][i])*9.8066) > 3){ // hard coded value for quantization of movement in y-axis
      return 1; // moved in y direction
    }
    else if (((imu.calcAccel(accelVals[2][i])*9.8066) > 11) | ((imu.calcAccel(accelVals[2][i])*9.8066) < 7)){ // hard coded values for quantization of movement in z-axis
      return 2; // moved in z direction
    }
  }
  return 3; // did not move
}

void blinkGreen(){
  digitalWrite(LEDRIGHT, HIGH);
  delay(100);                       
  digitalWrite(LEDRIGHT, LOW);    
  delay(100);
  digitalWrite(LEDRIGHT, HIGH);
  delay(100);                       
  digitalWrite(LEDRIGHT, LOW);    
  delay(100);
}

void blinkRed(){
  digitalWrite(LEDWRONG, HIGH);
  delay(100);                       
  digitalWrite(LEDWRONG, LOW);    
  delay(100);
  digitalWrite(LEDWRONG, HIGH);
  delay(100);                       
  digitalWrite(LEDWRONG, LOW);    
  delay(100);
}

void printKey(){
  for (int i = 0; i < 3; i = i + 1){
    Serial.println(keyVals[i]);
  }
}
