#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

LSM9DS1 imu;

#define PRINT_CALCULATED
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

#define DECLINATION -13.68 // Declination (degrees) in Shelton, CT.

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float axS, float ayS, float azS, float mxS, float myS, float mzS);
void blinkReg(int times)
void blinkSlow(int times)
void blinkLong(int howlong)

float accelPre[3][10]; //2D array for smoothing accel values
float magPre[3][10]; //2D array for smoothing mag values
unsigned char currVal = 0; //current index for changing

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Wire.begin();
  if (imu.begin() == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    while (1);
  }                    
}

void loop()
{
  // Update the sensor values whenever new data is available
  if ( imu.accelAvailable() )
  {
    imu.readAccel();
    accelPre[0][currVal] =  imu.ax;
    accelPre[1][currVal] =  imu.ay;
    accelPre[2][currVal] =  imu.az;
  }
  if ( imu.magAvailable() )
  {
    imu.readMag();
    magPre[0][currVal] =  imu.mx;
    magPre[1][currVal] =  imu.my;
    magPre[2][currVal] =  imu.mz;
  }

  if (currVal == 9){currVal = 0;}
  else {currVal = currVal + 1;}

  if ((lastPrint + PRINT_SPEED) < millis())
  {
    float avgAccel[3];
    float avgMag[3];

    for (int i = 0; i < 3; i = i+1){
      avgAccel[i] = (accelPre[i][0] + accelPre[i][1] + accelPre[i][2]+accelPre[i][3] + accelPre[i][4] + accelPre[i][5]+accelPre[i][6] + accelPre[i][7] + accelPre[i][8]+accelPre[i][9])/10;
      avgMag[i] = (magPre[i][0] + magPre[i][1] + magPre[i][2]+magPre[i][3] + magPre[i][4] + magPre[i][5]+magPre[i][6] + magPre[i][7] + magPre[i][8]+magPre[i][9])/10;
    }
                
    //printAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz);

    printAttitude(avgAccel[0], avgAccel[1], avgAccel[2], -avgMag[0], -avgMag[1], avgMag[2]);

    Serial.println();

    lastPrint = millis(); // Update lastPrint time
  }
}

void blink(int times){
  for (int i = 0; i <= times; i = i + 1){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
}

// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
void printAttitude(float axS, float ayS, float azS, float mxS, float myS, float mzS)
{

  //for smoothed
  float rollS = atan2(ayS, azS);
  float pitchS = atan2(-axS, sqrt(ayS * ayS + azS * azS));
  float quantPitch;
  float quantRoll;

  float headingS;
  float quantHeading;
  if (myS == 0)
    headingS = (mxS < 0) ? PI : 0;
  else
    headingS = atan2(mxS, myS);

  headingS -= DECLINATION * PI / 180;

  if (headingS > PI) headingS -= (2 * PI);
  else if (headingS < -PI) headingS += (2 * PI);

  // Convert everything from radians to degrees:
  headingS *= 180.0 / PI;
  pitchS *= 180.0 / PI;
  rollS  *= 180.0 / PI;

  //quantizing pitch
  if ((pitchS < 90) && (pitchS > 60)){
    quantPitch = 1;
  }
  else if ((pitchS < -60) && (pitchS > -90)){
    quantPitch = -1;
  }
  else {
    quantPitch = 0;
  }

  //quantizing roll
  if ((rollS < 105) && (rollS > 70)){
    quantRoll = 1;
  }
  else if ((rollS < -75) && (rollS > -105)){
    quantRoll = -1;
  }
  else {
    quantRoll = 0;
  }

  //quantizing heading (yaw)
  if ((headingS < 5) && (headingS > -25)){
    quantHeading = 1;
  }
  else if ((headingS < -140) && (headingS > -160)){
    quantHeading = -1;
  }
  else {
    quantHeading = 0;
  }
//  Serial.print(pitchS, 2);
//  Serial.print(" ");
//  Serial.print(rollS, 2);
//  Serial.print(" ");
//  Serial.print(headingS, 2);
//  Serial.print(" ");

  Serial.print(quantPitch, 2);
  Serial.print(" ");
  Serial.print(quantRoll, 2);
  Serial.print(" ");
  Serial.print(quantHeading, 2);
}

void blinkReg(int times){
  for (int i = 0; i < times; i = i + 1){
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(150);                       
    digitalWrite(LED_BUILTIN, LOW);    
    delay(150);
  }
}

void blinkSlow(int times){
  for (int i = 0; i < times; i = i + 1){
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(500);                       
    digitalWrite(LED_BUILTIN, LOW);    
    delay(500);
  }
}

void blinkLong(int howlong){
  digitalWrite(LED_BUILTIN, HIGH);   
    delay(howlong*1000);                       
    digitalWrite(LED_BUILTIN, LOW);    
    delay(1000);
}
