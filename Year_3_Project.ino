#include <LiquidCrystal_I2C.h>

#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const int SD_LED2 = 9;
const int Button = 2;
bool ledState = false;

int ErrorCode;

const int BUTTON_W = 4;
const int BUTTON_E = 5;
const int BUTTON_S = 6;

bool wristFlag = false, elbowFlag = false, shoulderFlag = false, Recordingflag = false;

float ROM, maxroll;
static bool offset_set = false;
static bool offset_roll_set = false;


String Tests_W[] = {"Flexion", "Extension"};
String Tests_E[] = {"Flexion", "Extension", "Pronation", "Supination"};
String Tests_S[] = {"Flexion", "Extension", "Adduction", "Abduction"};

int index_W = 0, index_E = 0, index_S = 0;

Adafruit_MPU6050 mpu;
File myFile;
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup(void)
{
  Serial.begin(115200);

  while (!Serial)
  {
    delay(10); // will pause Zero untill serial monitor is open
  }

  // Try to initialize
  if (!mpu.begin())
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
  
  lcd.init();
  lcd.backlight();

  attachInterrupt(digitalPinToInterrupt(Button), ISR0, FALLING); // Trigger on button press
  
  initSD();

  initMPU();

  Serial.println("");
  delay(500);

  startUp();

}

void loop() {
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

  float Angle = -atan2(-AccelX, AccelZ) * 180.0 / PI;
  float roll = atan(AccelY / sqrt(AccelX * AccelX + AccelZ * AccelZ)) * 180.0 / PI;


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

  if(Angle_norm >= ROM)
  {
    ROM = Angle_norm;
  }

  if(rollnorm >= maxroll)
  {
    maxroll=rollnorm;
  }
  lcd.setCursor(0, 0);
  lcd.print("X:"); lcd.print(AccelX,1);

  lcd.setCursor(7, 0);
  lcd.print("Y:"); lcd.print(AccelY,1);

  lcd.setCursor(0, 1);
  lcd.print("Z:"); lcd.print(AccelZ,1);

 
  
  Serial.print(" Tilt: "); Serial.print(Angle_norm); Serial.print("°  "); 
  Serial.print("Roll: "); Serial.print(rollnorm);  
  
  lcd.setCursor(7, 1); 
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

 /*
  if(Recordingflag == true)
  {
    myFile.print("Angle ");
    myFile.print(Angle_norm); myFile.print(" Roll ");
    myFile.println(rollnorm);
  }
  */

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
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
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