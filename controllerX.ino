#include <WiFi.h>
#include "MPU9250.h"

MPU9250 IMU(Wire, 0x68);
int status;
const int weight = 5 ;
float temp_X[2][10];
float temp_Y[2][10];

char ssid[] = "HelloCar";
char pass[] = "99eleven";
IPAddress server(192, 168, 4, 15);
WiFiClient client;
int led = 2, c = 0;
float of[2];
bool allset = false;

void setup()
{
  Serial.begin(115200);
  for (int i = 0; i < weight; i++)
  {
    temp_X[0][i] = 0;
    temp_X[1][i] = 0;
    temp_Y[0][i] = 0;
    temp_Y[1][i] = 0;
  }

  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while (1) {}
  }

  WiFi.begin(ssid, pass);           // connects to the WiFi AP
  Serial.println();
  Serial.println("Connection to the AP");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    blink(500);
  }
  Serial.println();
  Serial.println("Connected");
  Serial.print("LocalIP:"); Serial.println(WiFi.localIP());
  Serial.print("Gateway:"); Serial.println(WiFi.gatewayIP());
  pinMode(led, OUTPUT);
}

void loop()
{
  int i;
  while (!client.connected())
    client.connect(server, 80);
  float value_x = 0;
  float value_y = 0;

  IMU.readSensor();

  for (i = weight - 2; i >= 0; i--)
  {
    temp_X[0][i + 1] = temp_X[0][i];
    temp_X[1][i + 1] = temp_X[1][i];
    temp_Y[0][i + 1] = temp_Y[0][i];
    temp_Y[1][i + 1] = temp_Y[1][i];
  }
  temp_X[0][0] = IMU.getAccelX_mss();
  temp_X[1][0] = IMU.getGyroX_rads();
  temp_Y[0][0] = IMU.getAccelY_mss();
  temp_Y[1][0] = IMU.getGyroY_rads();
  for ( i = 0; i < weight; i++)
  {
    value_x += 0.7 * temp_X[0][i] + 0.3 * temp_X[1][i];
    value_y += 0.7 * temp_Y[0][i] + 0.3 * temp_Y[1][i];
  }
  value_x /= weight;
  value_y /= weight;


  if (c < 40) {
    if (c == 0) {
      of[0] = value_x;
      of[1] = value_y;
    }
    Serial.println("yo===yo");
    //calibrate
    of[0] = (of[0] + value_x) / 2.0;
    of[1] = (of[1] + value_y) / 2.0;
    c++;
    blink(100);
  }
  else {
    allset = true;
    digitalWrite(led, HIGH);
  }

  value_x = value_x - of[0];
  value_y = value_y - of[1];

  // PROCESSING IN CONTROLLER PART
  
      if(value_x>4)value_x = 4;  if(value_x<-4)value_x = -4;
      if(value_y>4)value_y = 4;  if(value_y<-4)value_y = -4;
  
      if(value_x>-0.3 && value_x<0.3)value_x = 0;
      if(value_y>-0.3 && value_y<0.3)value_y = 0;
        
      value_x = value_x>0?map(value_x, 0, 4, 0, 1023):map(value_x, -4, 0, -1023, 0);
      value_y = value_y>0?map(value_y, 0, 4, 0, 1023):map(value_y, -4, 0, -1023, 0);
  
  if (allset) {
    String enc_val = (String)value_x + " " + (String)value_y;
    Serial.println(enc_val);
    char x;
    while(true){
      if(!Serial.available()>0);
      else{
        x = Serial.read();
        break;
      }
    }
    //client.println(enc_val);
    client.println("600 600");
  }
}
void blink(int t) {
  digitalWrite(led, HIGH);
  delay(t);
  digitalWrite(led, LOW);
  delay(t);
}
