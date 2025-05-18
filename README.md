Code for a motor function detection system for cerebral palsy.

In this repository both the Arduino code as well as the Matlab code of the motor detection system can be found. The specific code is related to the project entitled “Quantitative analysis on hemiplegic cerebral palsy motor functions.”\\
Description

The Arduino code was tasked with setting up both the digital and physical user interface by setting up 4 push buttons as well as a digital LCD display. The Arduino was further tasked by initialising an MPU-6050 accelerometer along with an SD card data logging shield. The overall design enabled the user to visualise and record the current acceleration as well as the current max rang of motion for this movement. The recording (including stopping and starting) and moving to the next test is all controlled by the single interrupt button. Once all tests have been completed all relevant data has been written to the SD card and one is then able to plug that data into the Matlab code\\

The Matlab code was tasked with reading the individual files present on the SD card. After reading the data it then is capable of creating three different bar graphs for each of the effected regions tested. This was to enable non-technical practitioners to understand and interpret the patient’s max range of motion in order to determine a personalised treatment plan for the patient. Furthermore, the code was also tasked with calculating the patients % decrease in range of motion when compared to set of normal values which could then be used to calculate the patients MAS (Modified Ashworth Scale) score for each of their individual tests.\\

Software

In order for both codes to run the Arduino IDE and the Matlab IDE from MathWorks must be installed. Furthermore, for the Arduino IDE open source libraries such as; the Adafruit MPU-6050, LiquidCrystal and the SD libraries were used in the development of this system.\\

References

“Adafruit/Adafruit_MPU6050: Arduino Library for MPU6050.” GitHub, 15 Nov. 2023, github.com/adafruit/Adafruit_MPU6050. Accessed 10 February 2025.

“Arduino-Libraries/SD: SD Library for Arduino.” GitHub, 15 July 2024, github.com/arduino-libraries/SD. Accessed 10 February 2025.

“Johnrickman/LiquidCrystal_I2C: LiquidCrystal Arduino Library for the DFRobot I2C LCD Displays.” GitHub, 7 Mar. 2016, github.com/johnrickman/LiquidCrystal_I2C. Accessed 15 February 2025.



