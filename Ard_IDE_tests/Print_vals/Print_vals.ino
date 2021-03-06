
// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

// SDO_XM and SDO_G are both pulled high, so our addresses are:
// #define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW
// #define LSM9DS1_AG 0x6B // Would be 0x6A if SDO_AG is LOW

#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

#define DECLINATION -13.68 // Declination (degrees) in Shelton, CT.

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float axS, float ayS, float azS, float mxS, float myS, float mzS, float settingOrigin[10][3], unsigned char index);
void printArray(float settingOrigin[10][3]);

float accelPre[3][10]; //2D array for smoothing accel values
float magPre[3][10]; //2D array for smoothing mag values

float settingOrigin[10][3]; //2D array for storing averaged values for setting origin
float avgOrigin[3]; //array to hold final averaged stationary positions at origin

unsigned char currVal = 0; //current index for changing
unsigned char index = 0; //indexing for origin setting

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  if (imu.begin() == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1);
  }
}

void loop()
{
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();   
  }
  if ( imu.accelAvailable() )
  {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();

    accelPre[0][currVal] =  imu.ax;
    accelPre[1][currVal] =  imu.ay;
    accelPre[2][currVal] =  imu.az;
  }
  if ( imu.magAvailable() )
  {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
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
      avgAccel[i] = (accelPre[i][0] + accelPre[i][1] + accelPre[i][2]+accelPre[i][3] + accelPre[i][4] + accelPre[i][5]+accelPre[i][6] + accelPre[i][7] + accelPre[i][8]+accelPre[i][9])/3;
      avgMag[i] = (magPre[i][0] + magPre[i][1] + magPre[i][2]+magPre[i][3] + magPre[i][4] + magPre[i][5]+magPre[i][6] + magPre[i][7] + magPre[i][8]+magPre[i][9])/10;
    }

    if(index == 10){
      Serial.println("triggered==10");
      index = 0;
      Serial.println(index);
      printArray(settingOrigin);
    }
    else {
      Serial.println(index);
      index = index + 1;
    }

    // Print the heading and orientation for fun!
    // Call print attitude. The LSM9DS1's mag x and y
    // axes are opposite to the accelerometer, so my, mx are
    // substituted for each other.
    printAttitude(avgAccel[0], avgAccel[1], avgAccel[2], -avgMag[0], -avgMag[1], avgMag[2], settingOrigin, index);

    Serial.println();

    lastPrint = millis(); // Update lastPrint time
  }
}



// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
void printAttitude(float axS, float ayS, float azS, float mxS, float myS, float mzS, float settingOrigin[10][3], unsigned char index)
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

  
  Serial.print("P:");
  Serial.print(pitchS, 2);
  Serial.print(" R:");
  Serial.print(rollS, 2);
  Serial.print(" Y:");
  Serial.print(headingS, 2);
  Serial.print(" ");

  settingOrigin[index][0] = pitchS;
  settingOrigin[index][1] = rollS;
  settingOrigin[index][2] = headingS;
}

void printArray(float settingOrigin[10][3])
{
  float avgP = 0;
  float avgR = 0;
  float avgH = 0;
  for(int i = 0; i < 10; i = i + 1){
    avgP = avgP + settingOrigin[i][0];
    avgR = avgP + settingOrigin[i][1];
    avgH = avgP + settingOrigin[i][2];
  }

  avgP = avgP/10;
  avgR = avgR/10;
  avgR = avgR/10;

  avgOrigin[0] = avgP;
  avgOrigin[1] = avgR;
  avgOrigin[2] = avgH;

  Serial.print("avgP:");
  Serial.print(avgOrigin[0], 2);
  Serial.print(" avgR:");
  Serial.print(avgOrigin[1], 2);
  Serial.print(" avgY:");
  Serial.print(avgOrigin[2], 2);
  Serial.print(" ");
}
