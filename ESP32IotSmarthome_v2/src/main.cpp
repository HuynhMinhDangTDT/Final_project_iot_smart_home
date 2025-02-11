#include <Arduino.h>
#include "EEPROM.h"
#include "WiFi.h"
#include "connectWiFi.h"
#include "Firebase.h"
#include "Infrared.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Filters.h>
int pin_SDA = 9;
int pin_SCL = 8;



char ssid[32];
char pass[32];
char quserid[32];
int capability_RL1 = 4;
int capability_RL2 = 5;
int capability_RL3 = 6;
int capability_RL4 = 7;
int capability_RL5 = 15;
int capability_RL6 = 16;
void setup() {
  Serial.begin(115200);
  // Serial2.begin(9600, SERIAL_8N1,16,17);
  Serial1.begin(9600, SERIAL_8N1,18,17);
  pinMode(capability_RL1,INPUT);
  pinMode(capability_RL2,INPUT);
  pinMode(capability_RL3,INPUT);
  pinMode(capability_RL4,INPUT);
  pinMode(capability_RL5,INPUT);
  pinMode(capability_RL6,INPUT);
  Wire.begin(pin_SDA, pin_SCL);
  intitreadcapacity_Relay();
  WiFi.disconnect();
  delay(1000);
  EEPROM.begin(96);
  
  
  Serial.println("READING EEPROM SSID");
  if(EEPROM.read(0)!=0){
    strcpy(ssid,EEPROM.readString(0).c_str());
    strcpy(pass,EEPROM.readString(32).c_str());
    strcpy(quserid, EEPROM.readString(64).c_str());
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  delay(1000);
  if(WiFi.waitForConnectResult()==WL_CONNECTED){
    Serial.println("CONNECTED");
    initLcd();
    initDHT(); //setup DHT temp 
    for (int i = 10; i <= 14 ; i++)
    {
      pinMode(i, OUTPUT);
      pinMode(21, OUTPUT);
    }
    
    connectFirebase();
  
  }
  else{
    Serial.println("SETUPap");
    setupAP();
  }

  Serial.println();
  Serial.println("Waiting.");


  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }  

  // xTaskCreate(initInfrared,"Motion",8192,NULL,2,NULL);
}
  

void loop() {

  readTemp(quserid);
  TurnLight(quserid);
  readcapacity(quserid);
  readcapacity_Relay(quserid);
}




