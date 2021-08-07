/***************************************************************************************************************
 * Home automation using NodeMCU ESP-12 Develop Kit V1.0
 *  Sensor data sent to Thingspeak Channel: https://thingspeak.com/channels/
 *  Sensor data sent to Blynk app
 *  Downloads, docs, tutorials: http://www.blynk.cc
 *  Blynk library is licensed under MIT license
 *  Input Sensor :Gas sensor
 *               :Water Level Sensor
 *               :PIR sensor
 *               :Light sensor
 *               :Flame sensor
 *  Actuators    :Four channel relay module             
 *  Version 2.0.0 Blynk&TS - Developed by Mamrutha and team  
 *  Date : 25-04-2021 
 ********************************************************************************************************************************/

/* ESP */
#include <ESP8266WiFi.h>
WiFiClient client;

#include <BlynkSimpleEsp8266.h>
char auth[] = "xxxxxxxxxxx";
char ssid[] = "ABCD";
char pass[] = "Airtel@123";

/* Thinkspeak*/
const char* TS_SERVER = "api.thingspeak.com";
String TS_API_KEY ="AVE6FDRITJXXAFFC";


/* DHT22*/
#include "DHT.h"
#define DHTPIN D5 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float hum = 0;
float temp = 0;


/* TIMER */
#include <SimpleTimer.h>
SimpleTimer timer;

#define LdrPin D4
int   ldrVal = 0; 

#define PirSensorPin   D6 // PIR Sensor connected to digital pin
#define SmokeSensorPin D7 // Smoke sensor connected to digital pin
#define FlameSensorPin D8 // Flame sensor connected to digital pin

#define WaterLevelSensorPin A0 // Water level pin connected to Analog pin 
int Water_Level = 0;

const int maxLevel = 520;
const int midLevel = 450;
const int minLevel = 400;

int Flame_sensor = 0;
int Smoke_Sensor = 0;
int PIR_Sensor   = 0;


#define All_Auto     V4 
#define Light1_Man   V5       
#define Motor1_Man   V6       
#define Appliances1  V7 // Appliances1
#define Appliances2  V8 // Appliances2

int All_Auto_State   = 0;
int Light1_Man_State = 0;
int Motor1_Man_State = 0;
int App1_State       = 0;
int App2_State       = 0;

#define Relay_1_Light1       D0 // relay to light
#define Relay_2_Motor1       D1 // relay to motor pump
#define Relay_3_Appliances1  D2 // relay to home appliances 1
#define Relay_4_Appliances2  D3 // relay to home appliances 1

int Relay_1_Light1_Status       =   0;
int Relay_2_Motor1_Status       =   0; 
int Relay_3_Appliances1_Status  =   0; 
int Relay_4_Appliances2_Status  =   0; 

// Request the latest state from the server
BLYNK_CONNECTED() 
{
  Blynk.syncVirtual(All_Auto);
  Blynk.syncVirtual(Light1_Man);
  Blynk.syncVirtual(Motor1_Man);
  Blynk.syncVirtual(Appliances1);
  Blynk.syncVirtual(Appliances2);
}

// sync the recieved data from application to variables
BLYNK_WRITE(All_Auto) 
{
  All_Auto_State = param.asInt();
}


BLYNK_WRITE(Light1_Man) 
{
  Light1_Man_State = param.asInt();
}

BLYNK_WRITE(Motor1_Man) 
{
  Motor1_Man_State = param.asInt();
}

BLYNK_WRITE(Appliances1) 
{
  App1_State = param.asInt();
}

BLYNK_WRITE(Appliances2) 
{
  App2_State = param.asInt();
}

void setup()
{

   Serial.begin(9600);
   delay(10);
   Blynk.begin(auth, ssid, pass);
   pinMode(Relay_1_Light1, OUTPUT);
   pinMode(Relay_2_Motor1, OUTPUT);
   pinMode(Relay_3_Appliances1, OUTPUT);
   pinMode(Relay_4_Appliances2, OUTPUT);
   pinMode(DHTPIN, INPUT);
   pinMode(LdrPin, INPUT);
   pinMode(WaterLevelSensorPin, INPUT);
   pinMode(PirSensorPin, INPUT);
   pinMode(SmokeSensorPin, INPUT);
   pinMode(FlameSensorPin, INPUT);

   
   dht.begin();
   timer.setInterval(2000L, getInputData);
   timer.setInterval(5000L, sendUptime);
   timer.setInterval(19000L, sendDataTS);

}

void loop()
{
if (All_Auto_State == 1)
{    
  AutomateLight();
  AutomateMotor();
  FlamenSmoke();
}
else if (All_Auto_State == 0)
{
  light1();
  Motor1();
  App1();
  App2();
  FlamenSmoke();
}
else 
  {
    digitalWrite(Relay_1_Light1, LOW);
    digitalWrite(Relay_2_Motor1, LOW);
    digitalWrite(Relay_3_Appliances1, LOW);
    digitalWrite(Relay_4_Appliances2, LOW);
    Serial.println("Do nothing");
  } 
   
  timer.run(); // Initiates SimpleTimer
  Blynk.run();
  Blynk.virtualWrite(V3, Water_Level);
}

void AutomateLight()
{
  
   
    if(PIR_Sensor == 1 && ldrVal == 0 )
    {
      digitalWrite(Relay_1_Light1, HIGH); // turn on relay 1
      Serial.println("Auto Light1 ON");
      Blynk.virtualWrite(Light1_Man, 1); 
    }
    if (PIR_Sensor == 0 && ldrVal == 1)
    
    {
        digitalWrite(Relay_1_Light1, LOW); // turn off relay 1
        Serial.println("Auto Light1 OFF");
        Blynk.virtualWrite(Light1_Man, 0); 
    } 
}
  
  
  
 void AutomateMotor()
{
  if (Water_Level <= minLevel)
     {
      digitalWrite(Relay_2_Motor1, HIGH); // turn on relay 2
      Serial.println("Motor1 ON Auto");
      Blynk.virtualWrite(Motor1_Man, 1);   
     }
 if (Water_Level >= maxLevel)
     {
      digitalWrite(Relay_2_Motor1, LOW); // turn OFF relay 2
      Serial.println("Motor1 OFF Auto");
      Blynk.virtualWrite(Motor1_Man, 0);   
     }
  } 

void light1()
{
      if(All_Auto_State == 0 && Light1_Man_State == 1)
      {
      digitalWrite(Relay_1_Light1, HIGH); // turn on relay 1
      Serial.println("Manual Light1 ON");
      Blynk.virtualWrite(Light1_Man, 1);
      delay(100);
      }
      else if (All_Auto_State == 0 && Light1_Man_State == 0)
      {
      digitalWrite(Relay_1_Light1, LOW); // turn off relay 1
      Serial.println("Manual Light1 OFF"); 
      Blynk.virtualWrite(Light1_Man, 0);
      delay(100);
      }
      delay(100);
  
}
void  Motor1()
{
      if(All_Auto_State == 0 && Motor1_Man_State == 1)
      {
      digitalWrite(Relay_2_Motor1, HIGH); // turn on relay 2
      Serial.println("Manual Motor1  ON");
      Blynk.virtualWrite(Motor1_Man, 1); 
      delay(100);
      }
      else if (All_Auto_State == 0 )//&& Motor1_Man_State == 0
      {
      digitalWrite(Relay_2_Motor1, LOW); // turn off relay 2
      Serial.println("Manual Motor1  OFF");  
      Blynk.virtualWrite(Motor1_Man, 0); 
      delay(100);
      }
      delay(100);
}
  
 void App1()
{
   if(All_Auto_State == 0 && App1_State == 1 )
      {
      digitalWrite(Relay_3_Appliances1, HIGH); // turn on relay 3
      Serial.println("Manual appl 1 ON");
      }
      else if (All_Auto_State == 0 && App1_State == 0 )
      {
      digitalWrite(Relay_3_Appliances1, LOW); // turn off relay 3
      Serial.println("Manual appl 1 OFF");  
      }
      delay(100);
}
  
void  App2()
{
  if(All_Auto_State == 0 && App2_State == 1)
      {
      digitalWrite(Relay_4_Appliances2, HIGH); // turn on relay 4
      Serial.println("Manual appl 2 ON");
 //     Blynk.virtualWrite(Appliances2,1);
 //     delay(100);
      }
      else if(All_Auto_State == 0 && App2_State == 0)
      {
      digitalWrite(Relay_4_Appliances2, LOW); // turn off relay 4
      Serial.println("Manual appl 2 OFF");  
 //     Blynk.virtualWrite(Appliances2,0);
//      delay(100);
      }
      delay(100);
}
  
void  FlamenSmoke()
 {
  if (Flame_sensor == LOW )
  {
    Blynk.notify("Flame Detected !!!"); 
    Serial.println("Flame Detected !!!"); 
  }
  if (Smoke_Sensor == LOW)
  {
    Blynk.notify("Smoke Detected !!!"); 
    Serial.println("Smoke Detected !!!"); 
    
  }
 }

/***************************************************
 * Get DHT data
 **************************************************/
void getInputData(void)
{ 
  temp          = dht.readTemperature();
  hum           = dht.readHumidity();
  ldrVal        = !digitalRead(LdrPin);
  Water_Level   = analogRead(WaterLevelSensorPin);
  Flame_sensor  = digitalRead(FlameSensorPin);
  Smoke_Sensor  = digitalRead(SmokeSensorPin);
  PIR_Sensor    = digitalRead(PirSensorPin);  
}

/***************************************************
 * Send DHT data to Blynk
 **************************************************/
void sendUptime()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V0, temp); //virtual pin V10
  Blynk.virtualWrite(V1, hum); // virtual pin V11
  Blynk.virtualWrite(V2, ldrVal);
  
}

/**************************** ***********************
 * Sending Data to Thinkspeak Channel
 **************************************************/
void sendDataTS(void)
{
   if (client.connect(TS_SERVER, 80)) 
   { 
     String postStr = TS_API_KEY;
     postStr += "&field1=";
     postStr += String(temp);
     postStr += "&field2=";
     postStr += String(hum);
     postStr += "&field3=";
     postStr += String(Flame_sensor);  
     postStr += "&field4=";
     postStr += String(Smoke_Sensor);  
     postStr += "&field5=";
     postStr += String(Water_Level);   
//     postStr += "&field6=";
//     postStr += String(Relay_2_Motor1_Status);
     postStr += "\r\n\r\n";
   
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: " + TS_API_KEY + "\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
     delay(1000); 
   }
   client.stop();
}
