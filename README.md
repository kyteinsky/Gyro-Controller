# Gyro-Controller (Gyro Bars)
This is the Gyro controller project. All the files and images related to the project are shared here! Thanks.

Here we control a car (for demonstration purposes) using our gyro bars.

###  VERSION 0.3  *****************************************************************************************


> **car_2.ino file**

It contains the code for the __nodemcu__ board to used on the car, driven by a __L298N motor driver__
Its pin configurations are:
  - Enable A -> NOT USED
  - Enable B -> NOT USED
  - Input 1  -> 5
  - Input 2  -> 16
  - Input 3  -> 4
  - Input 4  -> 0
  
You can connect a suitable motor to the driver.

The car file receives the input data in from **two independent controllers** equipped with ESP32 Dev Module and InvenSense MPU9250 board each in two 3D printed enclosures.

> **controllerX.ino**

It contains code for the two controlling bars using __esp32__ board fitted with __MPU9250__ each for orientation approximation of the bar.

Pin configurations are as follows:
  - VCC, GND -> you know where to connect them, yet connect them to **3.3V** and **GND pins** although a voltage regulator is integrated onboard.
  - SDA pin (on MPU9250 board) -> SDA pin no. 21 (on ESP32 board)
  - SCL pin (on MPU9250 board) -> SCL pin no. 22 (on ESP32 board)
  
### First power up the car, then the controllers. Wait for 10 seconds (appx.) holding the bars at a comfortable position for calibration purposes.
### Enjoy!!
