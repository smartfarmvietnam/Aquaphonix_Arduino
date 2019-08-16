#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27 , 16, 2);
/*
 # This sample code is used to test the pH meter V1.0.
 # Editor : YouYou
 # Ver    : 1.0
 # Product: analog pH meter
 # SKU    : SEN0161
*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>

WiFiUDP ntpUDP;
//WiFi info 
const char* ssid = "AndroidHotspot3085";
const char* password = "00000000";
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String humi;
String times;


//GMT +7 = 25200
const long utcOffsetInSeconds = 25200;
NTPClient timeClient(ntpUDP , "pool.ntp.org" , utcOffsetInSeconds);

 void SensingLCD(int moisture){
  Serial.print("Moisture Sensor Value:");
  Serial.println(moisture);
  delay(2000);
} 

  String TimeChecker(){
   timeClient.update();
   const char* day = daysOfTheWeek[timeClient.getDay()];
   String hour =(String) timeClient.getHours() + ":";
   String minutes =(String) timeClient.getMinutes()+ ":";
   String seconds =  (String)timeClient.getSeconds();
  
   String times = hour + minutes + seconds;

   return times;
  }
//HTTP 통신 
  void NetWorkTask(String ph, String times){
    if (WiFi.status() == WL_CONNECTED) { 
       HTTPClient http;  
       http.begin("http://192.168.43.223:3000/aqua_ph?ph="+ph+"&ph_date="+times+"");
       int httpCode = http.GET();                                                                 
      
      if (httpCode == HTTP_CODE_OK) { 
        
        Serial.print("HTTP response code ");
        Serial.println(httpCode); 
        String response = http.getString();   
        Serial.println(response);      
      }else{
        Serial.println("DO Not Request!!!");
      }
    http.end();   //Close connection
  }
}
#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.08            //deviation compensate
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;
String ph;

double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
void setup()
{
  lcd.clear();
  lcd.init();
  lcd.backlight();
  pinMode(LED,OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
}
  Serial.println("WiFi Connect!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
}
void loop()
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
 
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Voltage:");
    lcd.print(voltage,2);
    Serial.print("Voltage:");
    Serial.print(voltage,2);
    lcd.setCursor(0,1);
    lcd.print("pH value:");
    lcd.println(pHValue,2);
    Serial.print("pH value:");
    Serial.println(pHValue,2);
    digitalWrite(LED,digitalRead(LED)^1);
    printTime=millis();
    times = TimeChecker();
    ph = (float)pHValue; 
    NetWorkTask(ph, times);

  }
