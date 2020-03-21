// Include necessary libraries
#include <ESP8266WiFi.h>

// DECLARATION OF PIN NUMBERS FOR MOTOR DRIVER
//#define ena 12
#define in1 5
#define in2 16
//#define enb 14
#define in3 4
#define in4 0

// FUNCTION DECLARATION
void move_car(String);

// CONFIGURING THE WI-FI SETTINGS && DECLARATION OF VARIABLES
WiFiServer server(80);// Open server at port 80 using TCP
IPAddress IP(192, 168, 4, 15);// FIXED IP
IPAddress mask = (255, 255, 255, 0);// Subnet mask for the network
byte ledPin = LED_BUILTIN;
float X[2], Y[2];// Values received from individual controllers
int x, y;// Scaled values of x and y to run the motor (0 to 1023)
int st = 0;// Stops motor from running till both clients are not connected
float fac = 0.833333;// Weighted mean factor
WiFiClient client[2];
String data[2];



void setup()
{
  //pinMode(ena, OUTPUT);
  //pinMode(enb, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  // HIGH AND LOW VALUES ARE INVERTED IN NODEMCU
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, HIGH);
  digitalWrite(ledPin, LOW);
  
  // STARTING THE WI-FI AS ACCESS POINT AND SERIAL AT BAUD RATE 115200
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("HelloCar", "99eleven");// SSID AND PASSWD
  WiFi.softAPConfig(IP, IP, mask);
  server.begin();//INITIALIZE THE SERVER IN THE CAR
  Serial.println();
  // PRINT OUT DETAILS TO TELL THE USER THAT THE SERVER IS READY
  Serial.println("accesspoint******************************");
  Serial.println("Server started.");
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  Serial.print("MAC:"); Serial.println(WiFi.softAPmacAddress());
  conn();// TRY TO CONNECT TO CLIENTS
}

void conn(){
  while(true){
    client[0] = server.available();// Connection to the first client
    if (client[0]) {
      Serial.println("client 0 connected");
      client[1] = server.available();// Try for the second client if the first one is connected
      if (client[1]) {
        Serial.println("client 1 connected");
        st = 1;
        break;
      }
    }
  }
}

void rec(){
  for (int i=0 ; i<2 ; i++) {// for two clients
    if (client[i]!=NULL) {
      data[i] = client[i].readStringUntil('\r');// read incoming data as a string
        //Processing of the data
      X[i] = data[i].substring(0, data[i].indexOf(" ")).toFloat();
      Y[i] = data[i].substring(data[i].indexOf(" ")+1, data[i].length()).toFloat();

      if(data[i] == "600 600")Serial.println("Done");
      // BELOW PROCESSING IS DONE ON THE CONTROLLER BOARD
      
      /*if(X[i]>4)X[i] = 4;  if(X[i]<-4)X[i] = -4;
      if(Y[i]>4)Y[i] = 4;  if(Y[i]<-4)Y[i] = -4;
  
      if(X[i]>-0.3 && X[i]<0.3)X[i] = 0;
      if(Y[i]>-0.3 && Y[i]<0.3)Y[i] = 0;
        
      X[i] = X[i]>0?map(X[i], 0, 4, 0, 1023):map(X[i], -4, 0, -1023, 0);
      Y[i] = Y[i]>0?map(Y[i], 0, 4, 0, 1023):map(Y[i], -4, 0, -1023, 0);*/
      
      // DEBUGGING PURPOSES -- REMOVE IF REQUIRED
      Serial.print("Y[i]=");
      Serial.println(Y[i]);
      Serial.print("X[i]=");
      Serial.println(X[i]);
    }
  }
}


void resetm(){// SET ALL VALUES TO LOW
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, HIGH);
}

void loop()
{
  rec();// Reception of datt
  merge();// Merge data from two controllers
  if(st==1)move_car();// If the two controllers are connected, move the car according to the values received
}

void merge(){
  /*if(abs(X[0] - X[1])>150)
    x = X[0]>X[1] ? (X[1]*fac+X[0]*(1-fac)) : (X[0]*fac+X[1]*(1-fac));
  else
    x = 0.5*(X[0]+X[1]);
  if(abs(Y[0] - Y[1])>150)
    y = Y[0]>Y[1] ? (Y[1]*fac+Y[0]*(1-fac)) : (Y[0]*fac+Y[1]*(1-fac));
  else
    y = 0.5*(Y[0]+Y[1]);
  x = 0.5*(X[0]+X[1]);
  y = 0.5*(Y[0]+Y[1]);*/
  
  if(abs(X[0]) - abs(X[1])>150 && (X[0]>0 && X[1]>0))// Weighted mean for greater precision in control
    x= X[0]>X[1] ? (X[1]*fac+X[0]*(1-fac)) : (X[0]*fac+X[1]*(1-fac));
  else
    x= 0.5*(X[0]+X[1]);

  // Other calculations based on position of the controller
  if(abs(Y[0]) - abs(Y[1])>150 && (Y[0]>0 && Y[1]>0))
    y= Y[0]>Y[1] ? (Y[1]*fac+Y[0]*(1-fac)) : (Y[0]*fac+Y[1]*(1-fac));
  else
    y= 0.5*(Y[0]+Y[1]);
}

//int x, y;//-1023 to 1023
void move_car()
{ 
  // See if data stream has been received correctly
        Serial.print("Move car: \tx=");
        Serial.print(x);
        Serial.print("\ty=");
        Serial.println(y);
  
  // Thresholding of the 'x' and 'y' values for testing purposes
  // Make use of the below commented lines of code for speed control
  if(x>400){
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  else if(x<-400){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }
  else if(y<-400){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  else if(y>400){
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }
  else resetm();
  
 
  // FIRST VARIANT OF SPEED CONTROL
  
  /*if(abs(x)<450){
    y = y>0 ? map(y, 0, 1023, 0, 250) : map(y, -1023, 0, -250, 0);
    analogWrite(ena, abs(x) + y>0? y : 0);
    analogWrite(enb, abs(x) + y>0? 0 : abs(y));
    
  }
  else if(abs(x) < 750){
    y = y>0 ? map(y, 0, 1023, 0, 200) : map(y, -1023, 0, -200, 0);
    analogWrite(ena, abs(x) + y>0? y : -50);
    analogWrite(enb, abs(x) + y>0? -50 : abs(y));
  }
  else{
    y = y>0 ? map(y, 0, 1023, 0, 200) : map(y, -1023, 0, -200, 0);
    analogWrite(ena, abs(x) + y>0? 0 : -250);
    analogWrite(enb, abs(x) + y>0? -250 : 0);
  }
  
    digitalWrite(in1, x>0 ? HIGH : LOW);
    digitalWrite(in2, x>0 ? LOW : HIGH);
    digitalWrite(in3, x>0 ? HIGH : LOW);
    digitalWrite(in4, x>0 ? LOW : HIGH);*/
  
  //************************************************************************************
  // SECOND VARIANT OF SPEED CONTROL -- NEEDS MODIFICATION
  
  
  
/*int i = 0;

  /*float x = a.substring(0, a.indexOf(" ")).toFloat();
  String k = a.substring(a.indexOf(" ") + 1);
  float y = k.substring(0, k.indexOf(" ")).toFloat();
  float accel_z = k.substring(k.indexOf(" ") + 1).toFloat();
  int speed_x, speed_y;
  int d_x, d_y;
  d_x = d_y = 0;
  speed_x = speed_y = 0;
  /*if (accel_x > 0) {
    speed_x = map(accel_x, 0, 10, 0, 255);
    d_x = 1;
  }
  if (accel_y > 0) {
    speed_y = map(accel_y, 0, 10, 0, 255);
    d_y = 1;
  }
  if (accel_x < 0) {
    speed_x = map(accel_x, -10, 0, 0, 255);
    d_x = -1;
  }
  if (accel_y < 0) {
    speed_y = map(accel_y, -10, 0, 0, 255);
    d_y = -1;
  }

  if (d_x == 1) {
    digitalWrite(m1_a, HIGH);
    digitalWrite(m1_b, LOW);
    digitalWrite(m2_a, HIGH);
    digitalWrite(m2_b, LOW);
    analogWrite(enb1, speed_x);
    analogWrite(enb2, speed_x);
    if (d_y == 1) {
      digitalWrite(m1_a, HIGH);
      digitalWrite(m1_b, LOW);
      digitalWrite(m2_a, LOW);
      digitalWrite(m2_b, LOW);
      analogWrite(enb1, speed_y);
    }
    if (d_y == -1) {
      digitalWrite(m1_a, LOW);
      digitalWrite(m1_b, LOW);
      digitalWrite(m2_a, HIGH);
      digitalWrite(m2_b, LOW);
      analogWrite(enb2, speed_y);
    }
  }


  if (d_x == -1) {
    digitalWrite(m1_b, HIGH);
    digitalWrite(m1_a, LOW);
    digitalWrite(m2_b, HIGH);
    digitalWrite(m2_a, LOW);
    analogWrite(enb1, speed_x);
    analogWrite(enb2, speed_x);
    if (d_y == 1) {
      digitalWrite(m1_b, HIGH);
      digitalWrite(m1_a, LOW);
      digitalWrite(m2_b, LOW);
      digitalWrite(m2_a, LOW);
      analogWrite(enb1, speed_y);
    }
    if (d_y == -1) {
      digitalWrite(m1_b, LOW);
      digitalWrite(m1_a, LOW);
      digitalWrite(m2_b, HIGH);
      digitalWrite(m2_a, LOW);
      analogWrite(enb2, speed_y);
    }*/
  }
