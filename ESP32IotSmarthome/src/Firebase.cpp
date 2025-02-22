#include "Firebase.h"
#include <Firebase_ESP_Client.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <cmath>
// #include "ModbusIP_ESP8266.h"
// #include "ModbusRTU.h"
// #include <Ticker.h>
// #include <Adafruit_MCP4725.h>
// #include <PZEM004Tv30.h>



//Define Firebase Data object
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

byte ByteArray[250];
int ByteData[20];

String led_state = "false";// LED State
String name_device = "false";// LED State
int led_gpio = 0, led_gpio_pin = 0;
bool signupOK = false;


// LiquidCrystal_I2C lcd(0x3f,16,2);
LiquidCrystal_I2C lcd(0x27,20,4); 

DHT dht(DHT11_PIN, DHTTYPE);

void initLcd()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
}

void connectFirebase()
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    config.database_url = DATABASE_URL;
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    }
    else 
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}


void initDHT() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DHT11_PIN, INPUT);
  dht.begin();

}

void initMobus() {
  Serial2.begin(9600, SERIAL_8N1,17,16);
}


void readTemp(const char* quserid)
{
    // float temp = dht.readTemperature();
    // float hum = dht.readHumidity();
    float temp;
    float hum;
    temp = 30;
    hum = 80;
    if (temp > 35 )
    {
      digitalWrite(BUZZER_PIN, HIGH); // turn on
      delay (5000);
      digitalWrite(BUZZER_PIN, LOW);
    }
    String urlReadTemp = String("Users/") + quserid + String("/DHT11/nhietdo");
    String urlReadHum  = String("Users/") + quserid + String("/DHT11/doam");
    if (Firebase.ready() ) 
    {
      if (Firebase.RTDB.setFloat(&fbdo, urlReadTemp , temp)){
        Serial.print("nhiet do: ");
        Serial.println(temp);
      } 
      if (Firebase.RTDB.setFloat(&fbdo, urlReadHum, hum)){
        Serial.print("do am: ");
        Serial.println(hum);
      }
      Serial.println();
    delay(1000);
  }
  lcd.clear();
  // check whether the reading is successful or not
  if (isnan(temp) || isnan(hum)) {
    Serial.println("failed to lcd");
    lcd.setCursor(0, 0);
    lcd.print("Failed");
  } else {
    Serial.println("success to lcd");
    // lcd.setCursor(0, 0);  // display position
    // lcd.print("Temp: ");
    // lcd.print(temp);    // display the temperature
    // lcd.print(" C");
    // lcd.setCursor(0, 1);  // display position
    // lcd.print("Humi: ");
    // lcd.print(hum);      // display the humidity
    // lcd.print(" %");
  }
}

void readcapacity(const char* quserid){
  // SV_after = mb2.Hreg(1);
  // frequency_after = mb2.Hreg(3);
  byte msg[] = {0x01,0x04,0x00,0x00,0x00,0x0A,0x70,0x0D};

  int i;
  int len=8; 

  ////// Sending Frame Modbus for Serial Port 2 
  /////// Envio de Trama Modbus por Puerto Serial #2

  Serial.println("ENVIO DATOS  -  SEND DATA");
  for(i = 0 ; i < len ; i++){
        Serial2.write(msg[i]); 
        Serial.print("[");
        Serial.print(i);
        Serial.print("]");
        Serial.print("=");   
        Serial.print(String(msg[i], HEX));      
  }
  len = 0;
  Serial.println();
  Serial.println();
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  //// -  2 step
  ////////// Reception  Frame 10 Registers (Input Registers) Modbus RTU - RS485 ////////////////////////
  /////////// Recepción de Trama 10 Registros (Input Registers) Modbus RTU - RS485 //////////////////////

  int a = 0;
  while(Serial2.available()) 
  {
  ByteArray[a] = Serial2.read();
  a++;
  }

  int b = 0;
  String registros;
      Serial.println("DATA RECEPTION  -  RECEPCIÓN DATOS");
      for(b = 0 ; b < a ; b++){      
        Serial.print("[");
        Serial.print(b);
        Serial.print("]");
        Serial.print("=");    
      
        registros =String(ByteArray[b], HEX);   
        Serial.print(registros);
        Serial.print(" ");         
    }
        Serial.println();
        Serial.println();
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  //// -  3 step
  //////// Procesamiento de Registros HEX  //////////////////////////////////////////////////////////
  //////// HEX Registers Processing ////////////////////////////////////// ////////////////////
  //////// Conversion de 2 Byte a 1 int - Conversion from 2 Byte to 1 int

  Serial.println("REGISTERS HEX");

  ByteData[0] = ByteArray[3] * 256 + ByteArray[4];
  Serial.println(ByteData[0],DEC);
  ByteData[1] = ByteArray[5] * 256 + ByteArray[6];
  Serial.println(ByteData[1],DEC);
  //ByteData[2] = ByteArray[7] * 256 + ByteArray[8];
  //Serial.println(ByteData[2],DEC);
  ByteData[3] = ByteArray[9] * 256 + ByteArray[10];
  Serial.println(ByteData[3],DEC);
  //ByteData[4] = ByteArray[11] * 256 + ByteArray[12];
  //Serial.println(ByteData[4],DEC);
  ByteData[5] = ByteArray[13] * 256 + ByteArray[14];
  Serial.println(ByteData[5],DEC);
  //ByteData[6] = ByteArray[15] * 256 + ByteArray[16];
  //Serial.println(ByteData[6],DEC);
  ByteData[7] = ByteArray[17] * 256 + ByteArray[18];
  Serial.println(ByteData[7],DEC);
  ByteData[8] = ByteArray[19] * 256 + ByteArray[20];
  Serial.println(ByteData[8],DEC);
  //ByteData[9] = ByteArray[21] * 256 + ByteArray[22];
  //Serial.println(ByteData[9],DEC);
  //ByteData[10] = ByteArray[23] * 256 + ByteArray[24];
  //Serial.println(ByteData[10],DEC);

  Serial.println();
  Serial.println();
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  //// -  4 step
  ///////// Securities Normalization ////////////////////////////////////// /////////////////////////
  /////////Normalizacion de Valores//////////////////////////////////////////////////////////////////

  float Voltage,Current,Power,Energy,Frequency,PowerFactor;
  Voltage     = ByteData[0] * 0.1;     // Tensión (80-260VAC)
  Current     = ByteData[1] * 0.001;   // Corriente (0-100A)
  Power       = ByteData[3] * 0.1;     // Potencia Activa (0-23000W)
  Energy      = ByteData[5] ;          // Potencia Acumulada (0-9999kWh)
  Frequency   = ByteData[7] * 0.1;     // Frecuencia (45-65Hz)
  PowerFactor = ByteData[8] * 0.01;    // Factor de Potencia (0.00 – 1.00)  

  Serial.println("MEDICIONES - MEASUREMENTS");
  Serial.print("Voltage ");
  Serial.println(Voltage);
  Serial.print("Current ");
  Serial.println(Current);
  Serial.print("Power ");
  Serial.println(Power);
  Serial.print("Energy ");
  Serial.println(Energy);
  Serial.print("Frequency ");
  Serial.println(Frequency);
  Serial.print("PowerFactor ");
  Serial.println(PowerFactor);
  Serial.println();
  delay(200);                      /// delay para permitir ver valores - delay to allow viewing values -
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  String urlReadTemp = String("Users/") + quserid + String("/capacity/CongSuatTieuThu");
  if (Firebase.ready() )
  {
    if (Firebase.RTDB.setFloat(&fbdo, urlReadTemp , Energy)){
      Serial.print("Cong suat tieu thu: ");
      Serial.println(Energy);
      lcd.setCursor(0, 0);  // display position
      // float multiplier = pow(100, decimalPlaces);
      lcd.print("P tieu thu:" + String(int(Energy)) + String(" KWh"));
    }
    Serial.println();
  }
  delay(1000);
}
void TurnLight(const char* quserid)
{
    if (Firebase.ready())
      {
        // for (int led_gpio_pin = 25; led_gpio_pin <= 27; led_gpio_pin++)
        // {
          for (int i = 0; i <= 2; i++)
          { 
              led_gpio = i;
              switch (i){
                case 0: led_gpio_pin = 25; break;
                case 1: led_gpio_pin = 26; break;
                case 2: led_gpio_pin = 27; break;
              }
              String urlDevice = String("Users/") + quserid + String("/device/") + i + String ("/status");
              String urlDevice_name = String("Users/") + quserid + String("/device/") + i + String ("/name");
              led_state = Firebase.RTDB.getString(&fbdo, urlDevice) ? fbdo.to<const char *>() : "false" ;
              name_device = Firebase.RTDB.getString(&fbdo, urlDevice_name) ? fbdo.to<const char *>() : "" ;
              Serial.println(urlDevice);
              if (led_gpio == i && led_state == "false") {
                  Serial.println(String("ESP32-GPIO ") + led_gpio_pin + String(" is OFF"));
                  lcd.setCursor(0, i+1);  // display position
                  lcd.print(String(name_device) + ":"+ String(" OFF"));
                  digitalWrite(led_gpio_pin, led_gpio == 0 ? LOW : HIGH);
              }
              else if (led_gpio == i && led_state == "true") {
                  Serial.println(String("ESP32-GPIO ") + led_gpio_pin + String(" is ON"));
                  lcd.setCursor(0, i+1);  // display position
                  lcd.print(String(name_device) + ":"+ String(" ON"));
                  digitalWrite(led_gpio_pin, led_gpio == 0 ? HIGH : LOW);
              }
              else {
                  Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, urlDevice) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
              }
          // }
        }
      
    }
}