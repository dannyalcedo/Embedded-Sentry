#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

LSM9DS1 imu;

#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

#define DECLINATION -13.68 // Declination (degrees) in Shelton, CT.

//Function definitions
void printSmoothAccel(float ax, float ay, float az);
void printRawAccel();

float accelPre[3][5]; //2D array for smoothing accel values
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

    accelPre[0][currVal] =  imu.ax;
    accelPre[1][currVal] =  imu.ay;
    accelPre[2][currVal] =  imu.az;
  }

  if (currVal == 4){currVal = 0;}
  else {currVal = currVal + 1;}

  if ((lastPrint + PRINT_SPEED) < millis())
  {
    float avgAccel[3];

    for (int i = 0; i < 3; i = i+1){
      avgAccel[i] = (accelPre[i][0] + accelPre[i][1] + accelPre[i][2]+accelPre[i][3] + accelPre[i][4])/5;
    }

//    printSmoothAccel(avgAccel[0], avgAccel[1], avgAccel[2]); // Print "A: ax, ay, az"
    printRawAccel();
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

void printSmoothAccel(float ax, float ay, float az) {
  Serial.print("A: ");
  float accelXms2 = imu.calcAccel(ax) * 9.8066;
  float accelYms2 = imu.calcAccel(ay) * 9.8066;
  float accelZms2 = imu.calcAccel(az) * 9.8066;
  Serial.print(accelXms2, 2);
  Serial.print(", ");
  Serial.print(accelYms2, 2);
  Serial.print(", ");
  Serial.print(accelZms2, 2);
}
