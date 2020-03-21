// ESP32 and MPU9250 used

// Include Wi-Fi and MPU9250 libraries
#include <WiFi.h>
#include "MPU9250.h"

MPU9250 IMU(Wire, 0x68);// Initialize the IMU sensor with the default address 0x68
int status;// Status of connection of the IMU sensor with ESP32 board
const int weight = 5 ;
float temp_X[2][10];
float temp_Y[2][10];

char ssid[] = "HelloCar";
char pass[] = "99eleven";
IPAddress server(192, 168, 4, 15);
WiFiClient client;
int led = 2, c = 0;// c acts as counter for calibration
float of[2];// Setting the offset for calibration in any orientation
bool allset = false;// Indicator if the connection and calibration has been made

void setup()
{
  Serial.begin(115200);
  //Initializing all values to zero
  for (int i = 0; i < weight; i++)
  {
    temp_X[0][i] = 0;
    temp_X[1][i] = 0;
    temp_Y[0][i] = 0;
    temp_Y[1][i] = 0;
  }

  status = IMU.begin();// Get status of connection of the IMU
  if (status < 0) {// Not connected or improperly connected
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while (1) {}// Freeze the code execution here
  }

  WiFi.begin(ssid, pass);           // connect to the WiFi AP
  Serial.println();
  Serial.println("Connection to the AP");
  while (WiFi.status() != WL_CONNECTED)// WAIT
  {
    Serial.print(".");
    blink(500);
  }
  Serial.println();
  Serial.println("Connected");
  // Print out th details for the connection
  Serial.print("LocalIP:"); Serial.println(WiFi.localIP());
  Serial.print("Gateway:"); Serial.println(WiFi.gatewayIP());
  pinMode(led, OUTPUT);
  
  // Connect to server hosted by client over TCP(port=80)
  while (!client.connected())
    client.connect(server, 80);
}

void loop()
{
  int i;
  // No need to check everytime for the connection
  /*while (!client.connected())
    client.connect(server, 80);*/
  
  // Our temp. storage
  float value_x = 0;
  float value_y = 0;

  // Read the sensor for data
  IMU.readSensor();

  //*************************************************************
  //IMU processing
  for (i = weight - 2; i >= 0; i--)
  {
    temp_X[0][i + 1] = temp_X[0][i];
    temp_X[1][i + 1] = temp_X[1][i];
    temp_Y[0][i + 1] = temp_Y[0][i];
    temp_Y[1][i + 1] = temp_Y[1][i];
  }
  // Get values from the accelerometer and gyroscope in X and Y axes --
  // CHANGE X and Y to DESIRED AXES according to need
  temp_X[0][0] = IMU.getAccelX_mss();
  temp_X[1][0] = IMU.getGyroX_rads();
  temp_Y[0][0] = IMU.getAccelY_mss();
  temp_Y[1][0] = IMU.getGyroY_rads();
  
  // Weighted mean 70% for the accelerometer and 30% for the gyroscope
  // Play around with these values for personal setting
  for ( i = 0; i < weight; i++)
  {
    value_x += 0.7 * temp_X[0][i] + 0.3 * temp_X[1][i];
    value_y += 0.7 * temp_Y[0][i] + 0.3 * temp_Y[1][i];
  }
  //**************************************************************
  value_x /= weight;
  value_y /= weight;

  // Calibration in the present orientation
  if (c < 40) {
    if (c == 0) {
      of[0] = value_x;
      of[1] = value_y;
    }
    Serial.println("yo===yo");// Marker for the stage of the code
    // Setting the offset by averaging present values
    of[0] = (of[0] + value_x) / 2.0;
    of[1] = (of[1] + value_y) / 2.0;
    c++;
    blink(100);
  }
  else {
    allset = true;// Green flag to send data to the car
    digitalWrite(led, HIGH);
  }

  // Adjust the orientation values as if considering an initial position was set at the beginning
  value_x = value_x - of[0];
  value_y = value_y - of[1];

  // PROCESSING IN CONTROLLER PART
  
      if(value_x>4)value_x = 4;  if(value_x<-4)value_x = -4;
      if(value_y>4)value_y = 4;  if(value_y<-4)value_y = -4;
  
      if(value_x>-0.3 && value_x<0.3)value_x = 0;
      if(value_y>-0.3 && value_y<0.3)value_y = 0;
        
      value_x = value_x>0?map(value_x, 0, 4, 0, 1023):map(value_x, -4, 0, -1023, 0);
      value_y = value_y>0?map(value_y, 0, 4, 0, 1023):map(value_y, -4, 0, -1023, 0);
  
  if (allset) {// If its a green flag
    String enc_val = (String)value_x + " " + (String)value_y;// Get the values in a string to be sent
    Serial.println(enc_val);
    client.println(enc_val);// Send the values
    
    /* TESTING SPEED IN STAGES
    
    char x;
    while(true){
      if(!Serial.available()>0);
      else{
        x = Serial.read();
        break;
      }
    }*/
    
  }
}
void blink(int t) {// Blink func.
  digitalWrite(led, HIGH);
  delay(t);
  digitalWrite(led, LOW);
  delay(t);
}
