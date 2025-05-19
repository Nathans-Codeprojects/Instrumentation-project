#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <Adafruit_MPU6050.h>   //All libraries needed
#include <Adafruit_Sensor.h>
#include <Wire.h>

const int SD_LED2 = 9;   //Led for recording
const int Button = 2;    //interrupt button
bool ledState = false;  //initial state of the led

int ErrorCode;

const int BUTTON_W = 4;   //3 yellow buttons for the wrist, elbow and shoulder files/tests
const int BUTTON_E = 5;
const int BUTTON_S = 6;

bool wristFlag = false, elbowFlag = false, shoulderFlag = false, Recordingflag = false;

float ROM, maxroll; //Max ROM for the 2 different calculations
static bool offset_set = false;     //bool in order to reset the max ROM
static bool offset_roll_set = false;


String Tests_W[] = {"Flexion", "Extension"};
String Tests_E[] = {"Flexion", "Extension", "Pronation", "Supination"};  //Arrays containing the test names for each effected region
String Tests_S[] = {"Flexion", "Extension", "Adduction", "Abduction"};

int index_W = 0, index_E = 0, index_S = 0;

Adafruit_MPU6050 mpu;
File myFile;              //class objects
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup(void)
{
  Serial.begin(115200);

  while (!Serial)
  {
    delay(10); // will pause Zero untill serial monitor is open
  }

  // Try to initialize
  if (!mpu.begin())   //initialize mpu-6050
  {
    ErrorCode = 1;
    error(ErrorCode);
  }

  Serial.println("MPU6050 Found");

  pinMode(SD_LED2, OUTPUT);

  pinMode(Button, INPUT_PULLUP);
  pinMode(BUTTON_W, INPUT_PULLUP);
  pinMode(BUTTON_E, INPUT_PULLUP);
  pinMode(BUTTON_S, INPUT_PULLUP);  
  
  lcd.init();    //initialize LCD screen
  lcd.backlight();

  attachInterrupt(digitalPinToInterrupt(Button), ISR0, FALLING); // attach interupt on button press falling edge
  
  initSD(); //initialize SD card shield

  initMPU(); //sets up mpu-6050 parameters

  Serial.println("");
  delay(500);

  startUp(); //inital loop to cycle untill an effected region has been selected

}

void loop() {
  //open the different files depending on what button was pressed
  if(wristFlag == true)
  {
    myFile = SD.open("WRIST.txt", FILE_WRITE);
  }
  else if(elbowFlag == true)
  {
    myFile = SD.open("ELBOW.txt", FILE_WRITE);
  }
  else if(shoulderFlag == true)
  {
    myFile = SD.open("SHOULDER.txt", FILE_WRITE);
  }

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float AccelX = a.acceleration.x;
  float AccelY = a.acceleration.y;
  float AccelZ = a.acceleration.z;

  //calculation of the roll and tilt angles
  float Angle = -atan2(-AccelX, AccelZ) * 180.0 / PI;
  float roll = atan(AccelY / sqrt(AccelX * AccelX + AccelZ * AccelZ)) * 180.0 / PI;


  //determining the current angle when the test is started to calculate the deviation from 0
  static float roll_offset;
  static float Angle_offset;

  if (!offset_roll_set)
  {
  roll_offset = roll;
  offset_roll_set = true;
  }  
  
  if (!offset_set) {
    Angle_offset = Angle;
    offset_set = true;
  }

  float Angle_relative = Angle - Angle_offset;
  float Angle_norm = abs(Angle_relative);  
  
  float relative_roll = roll - roll_offset;
  float rollnorm = abs(relative_roll);

  //storing the max ROM
  if(Angle_norm >= ROM)
  {
    ROM = Angle_norm;
  }

  if(rollnorm >= maxroll)
  {
    maxroll=rollnorm;
  }
  //displaying the current acceleration data and max ROM on the LCD screen
  lcd.setCursor(0, 0);
  lcd.print("X:"); lcd.print(AccelX,1);

  lcd.setCursor(7, 0);
  lcd.print("Y:"); lcd.print(AccelY,1);

  lcd.setCursor(0, 1);
  lcd.print("Z:"); lcd.print(AccelZ,1);

 
  
  Serial.print(" Tilt: "); Serial.print(Angle_norm); Serial.print("°  "); 
  Serial.print("Roll: "); Serial.print(rollnorm);  
  
  lcd.setCursor(7, 1); 
  //showing the roll ROM for pronation and supination otherwise shows the tilt max ROM
  if(index_E == 3 || index_E == 4)
  {
    lcd.print("ROM:"); lcd.print(maxroll);  
    Serial.print("ROMroll: "); Serial.println(maxroll);
  }
  else
  {
    lcd.print("ROM:"); lcd.print(ROM);
    Serial.print("ROM: "); Serial.println(ROM);
  }
  delay(500);
}


void initSD() 
{
  Serial.print("Initialising SD Card... ");  // Print a message indicating SD card initialization
  if (!SD.begin()) 
  {
    ErrorCode = 2;
    error(ErrorCode);
  }
  else 
  {
    Serial.println("Card Detected\n");  // If the SD card is detected, print a success message
  }
}

void initMPU()
{
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);  //initialize mpu-6050 filter at 21Hz
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);//initialize mpu-6050 accelerometer sensitivity at 2g
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);//initialize mpu-6050 groscope sensitivity at 250deg
}

void startUp()
{
  while(true)
  {

    lcd.setCursor(0, 0);
    lcd.print("Effected Region");

    lcd.setCursor(0, 1);
    lcd.print("S");

    lcd.setCursor(4, 1);
    lcd.print("E");

    lcd.setCursor(8, 1);
    lcd.print("W");

    if(digitalRead(BUTTON_W) == LOW)
    {
      wristFlag = true;
      lcd.clear();
      return;
    }
    else if(digitalRead(BUTTON_E) == LOW)
    {
      elbowFlag = true;
      lcd.clear();
      return;
    }
    else if(digitalRead(BUTTON_S) == LOW)
    {
      shoulderFlag = true;
      lcd.clear();
      return;
    }
  }
}

void ISR0() 
{  
  //opens correct file when intturput button is pressed
  if (wristFlag) 
  {
    myFile = SD.open("WRIST.txt", FILE_WRITE);
  } 
  else if (elbowFlag) 
  {
    myFile = SD.open("ELBOW.txt", FILE_WRITE);
  } 
  else if (shoulderFlag) 
  {
    myFile = SD.open("SHOULDER.txt", FILE_WRITE);
  }


  if(ledState) 
  {
    //ledstate is true when the intterupt button is pressed to end the test therefore, the led is turned off and the max ROM
    //is written to the SD card
    digitalWrite(SD_LED2, LOW);
    Recordingflag = false;
    ledState = false;

    if(index_E == 3 || index_E == 4)
    {
      myFile.print("ROMroll: "); myFile.println(maxroll);
    }
    else
    {
      myFile.print("ROM: "); myFile.println(ROM);
    }
  } 
  else 
  {
    //when recording should start the ROM is reset and the current test is printed to the SD card while incrementing the array
    //inorder to move the array to the following test
    digitalWrite(SD_LED2, HIGH);    
  
    ROM = 0;
    maxroll = 0;

    offset_set = false;
    offset_roll_set = false;

    Recordingflag = true;    
    ledState = true;

    myFile.print("TEST,  ");
    if(wristFlag)
    {
      myFile.println(Tests_W[index_W]);
      index_W++;

      if(index_W > 1)
      {
        index_W = 0;
      }
    }
    else if(elbowFlag)
    {
      myFile.println(Tests_E[index_E]);
      index_E++;

      if(index_E > 4)
      {
        index_E = 0;
      }
    }
    else if(shoulderFlag)
    {
      myFile.println(Tests_S[index_S]);
      index_S++;

      if(index_S > 3)
      {
        index_S = 0;
      }
    }
  }

  myFile.close();
}

//error function to display any error message depending on what has gone wrong in the setup of the code
void error(int code)
{

  if(code == 1)
  {
    Serial.println("MPU-6050 NOT FOUND");
  }
  else if(code == 2)
  {
    Serial.println("NO SD CARD DETECTED");
  }
  else
  {
    Serial.println("UNKNOWN ERROR");
  }

  while(true)
  {
    delay(1);
  }
}
