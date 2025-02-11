
#ifndef _Firebase_h_
#define _Firebase_h_

#define DHT11_PIN 38
#define DHTTYPE DHT11
#define BUZZER_PIN 47


#define API_KEY "AIzaSyCvcp70TewaKGHAGl0XMWsNsZiGRm_hVa4"

#define DATABASE_URL "https://iot-smart-home-a7c95-default-rtdb.asia-southeast1.firebasedatabase.app/" 


void initLcd();
void intitreadcapacity_Relay();
void connectFirebase();
void initDHT();
void readTemp(const char* quserid);
void TurnLight(const char * quserid);
void initMobus();
void readcapacity(const char* quserid);
void readcapacity_Relay(const char* quserid);

#endif 