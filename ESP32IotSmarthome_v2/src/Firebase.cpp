#include "Firebase.h"
#include <Firebase_ESP_Client.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Filters.h>
#include <ACS712.h>


#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <cmath>
// #include "ModbusIP_ESP8266.h"
// #include "ModbusRTU.h"
// #include <Ticker.h>
// #include <Adafruit_MCP4725.h>
// #include <PZEM004Tv30.h>

RunningStatistics inputStats;  // Đối tượng tính toán RMS


float testFrequency = 50;                     // nhà bạn dùng điện bao nhiêu Hz? Ở VN là 60Hz
float windowLength = 20.0/testFrequency;     // mỗi tín hiệu thu về cách nhau bao nhiêu thời gian
int sensorValue1 = 0;
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;
int sensorValue5 = 0;
int sensorValue6 = 0;
// float intercept = 1263151.1;  // Hiệu chỉnh
// float slope = -0.0015493;  // Hiệu chỉnh
// float intercept = -0.1310; //  cẩn sửa trong quá trình hiệu chuẩn
// float slope = 0.04099; // cẩn sửa trong quá trình hiệu chuẩn
// float current_amps;  // Dòng điện đo được

// float voltage = 225.10;  // Điện áp xoay chiều (V)
// float power;  // Công suất tức thời (W)
// float energyWh = 0.0;  // Năng lượng tiêu thụ (Wh)

unsigned long printPeriod = 1000;  // Thời gian in Serial (ms)
unsigned long previousMillis = 0;  // Lưu thời điểm in cuối
unsigned long lastUpdateTime = 0;  // Lưu thời gian đo trước đó
// float energyConsumedRL1 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)
// float energyConsumedRL2 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)
// float energyConsumedRL3 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)
// float energyConsumedRL4 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)
// float energyConsumedRL5 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)
// float energyConsumedRL6 = 0.0;  // Biến lưu tổng điện năng tiêu thụ (Wh)

#define NUM_SENSORS 6  // Số lượng cảm biến
float energyConsumed[NUM_SENSORS] = {0};  // Mảng lưu điện năng tiêu thụ cho từng cảm biến
// Thông số cho từng cảm biến
float zeroCurrentADC[NUM_SENSORS] = {2119.00, 2119.00, 2119.00, 2119.00, 2119.00, 2119.00}; // Cập nhật giá trị này nếu cần
float slope[NUM_SENSORS] = {-0.0015493, -0.0015493, -0.0015493, -0.0015493, -0.0015493, -0.0015493};  // Cập nhật hệ số chuyển đổi nếu cần

#define capability_RL1 4
#define capability_RL2 5
#define capability_RL3 6
#define capability_RL4 7
#define capability_RL5 15
#define capability_RL6 16

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
  Serial1.begin(9600, SERIAL_8N1,18,17);
}


void readTemp(const char* quserid)
{
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    // float temp;
    // float hum;
    // temp = 30;
    // hum = 80;
    // if (temp > 35 )
    // {
    //   digitalWrite(BUZZER_PIN, HIGH); // turn on
    //   delay (5000);
    //   digitalWrite(BUZZER_PIN, LOW);
    // }
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

  // Serial.println("ENVIO DATOS  -  SEND DATA");
  for(i = 0 ; i < len ; i++){
        Serial1.write(msg[i]); 
  //       Serial.print("[");
  //       Serial.print(i);
  //       Serial.print("]");
  //       Serial.print("=");   
  //       Serial.print(String(msg[i], HEX));      
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
  while(Serial1.available()) 
  {
  ByteArray[a] = Serial1.read();
  a++;
  }

  int b = 0;
  String registros;
      // Serial.println("DATA RECEPTION  -  RECEPCIÓN DATOS");
      for(b = 0 ; b < a ; b++){      
        // Serial.print("[");
        // Serial.print(b);
        // Serial.print("]");
        // Serial.print("=");    
      
        registros =String(ByteArray[b], HEX);   
        // Serial.print(registros);
        // Serial.print(" ");         
    }
        // Serial.println();
        // Serial.println();
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  //// -  3 step
  //////// Procesamiento de Registros HEX  //////////////////////////////////////////////////////////
  //////// HEX Registers Processing ////////////////////////////////////// ////////////////////
  //////// Conversion de 2 Byte a 1 int - Conversion from 2 Byte to 1 int

  // Serial.println("REGISTERS HEX");

  ByteData[0] = ByteArray[3] * 256 + ByteArray[4];
  // Serial.println(ByteData[0],DEC);
  ByteData[1] = ByteArray[5] * 256 + ByteArray[6];
  // Serial.println(ByteData[1],DEC);
  //ByteData[2] = ByteArray[7] * 256 + ByteArray[8];
  //Serial.println(ByteData[2],DEC);
  ByteData[3] = ByteArray[9] * 256 + ByteArray[10];
  // Serial.println(ByteData[3],DEC);
  //ByteData[4] = ByteArray[11] * 256 + ByteArray[12];
  //Serial.println(ByteData[4],DEC);
  ByteData[5] = ByteArray[13] * 256 + ByteArray[14];
  // Serial.println(ByteData[5],DEC);
  //ByteData[6] = ByteArray[15] * 256 + ByteArray[16];
  //Serial.println(ByteData[6],DEC);
  ByteData[7] = ByteArray[17] * 256 + ByteArray[18];
  // Serial.println(ByteData[7],DEC);
  ByteData[8] = ByteArray[19] * 256 + ByteArray[20];
  // Serial.println(ByteData[8],DEC);
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
  

  // sensorValue1 = analogRead(capability_RL1);  //đọc giá trị
  // sensorValue2 = analogRead(capability_RL2);  //đọc giá trị
  // sensorValue3 = analogRead(capability_RL3);  //đọc giá trị
  // sensorValue4 = analogRead(capability_RL4);  //đọc giá trị
  // sensorValue5 = analogRead(capability_RL5);  //đọc giá trị
  // sensorValue6 = analogRead(capability_RL6);  //đọc giá trị

  //----------------------------------------------------------------
  //Tìm ADC khi không có tải
  // int sum = 0;
  // int numSamples = 100;

  // for (int i = 0; i < numSamples; i++) {
  //     sum += sensorValue1;
  //     delay(10);
  // }

  // float zeroCurrentADC = sum / (float)numSamples;
  // Serial.print("Zero Current ADC: ");
  // Serial.println(zeroCurrentADC);
  //------------------------------------------------------------------

  unsigned long currentMillis = millis();
  float elapsedTime = (currentMillis - previousMillis) / 3600000.0; // Chuyển ms sang giờ
  previousMillis = currentMillis;
  for (int i = 0; i < NUM_SENSORS; i++) {
    int sensorValue = 0;  // Biến lưu giá trị cảm biến
    float power = 0;
    float current = 0;
    switch(i) {
        case 0: sensorValue = analogRead(capability_RL1); break;
        case 1: sensorValue = analogRead(capability_RL2); break;
        case 2: sensorValue = analogRead(capability_RL3); break;
        case 3: sensorValue = analogRead(capability_RL4); break;
        case 4: sensorValue = analogRead(capability_RL5); break;
        case 5: sensorValue = analogRead(capability_RL6); break;
    }
    String urlDevice = String("Users/") + quserid + String("/device/") + i + String ("/status");
    led_state = Firebase.RTDB.getString(&fbdo, urlDevice) ? fbdo.to<const char *>() : "false" ;
    // Serial.println(urlDevice);
    if (led_state == "true") {
      // Tính dòng điện từ giá trị ADC
      current = (sensorValue - zeroCurrentADC[i]) * slope[i];

      // Tính công suất (W)
      power = Voltage * abs(current);

      // Tính điện năng tiêu thụ (Wh)
      energyConsumed[i] += power * elapsedTime;
    }
    else {
        Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, urlDevice) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
    }
    

    // Lưu giá trị điện năng vào EEPROM sau mỗi 5 giây (hoặc thời gian bạn muốn)
    // if (currentMillis % 5000 == 0) {
    //     EEPROM.write(EEPROM_ADDR[i], energyConsumed[i]);  // Lưu giá trị vào EEPROM
    //     EEPROM.commit();  // Lưu giá trị vào bộ nhớ không thay đổi
    // }
    
    // Lưu dữ liệu công suất và năng tiêu thụ vào Firebase
    String urlPower = String("Users/") + quserid + String("/device/") + i + String("/power");
    String urlEnergy = String("Users/") + quserid + String("/device/") + i + String("/energyConsumption");

    // Gửi công suất và năng tiêu thụ lên Firebase
    Firebase.RTDB.setFloat(&fbdo, urlPower.c_str(), power);
    Firebase.RTDB.setFloat(&fbdo, urlEnergy.c_str(), energyConsumed[i]);

    // Hiển thị kết quả
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" - ADC: ");
    Serial.print(sensorValue);
    Serial.print(" | Current: ");
    Serial.print(current, 3);
    Serial.print(" A | Power: ");
    Serial.print(power, 3);
    Serial.print(" W | Energy: ");
    Serial.print(energyConsumed[i] / 1000, 3);
    Serial.println(" Wh");
  }
  // float zeroCurrentADCRL2 = 2119.00;
  // int rawValueRL2 = sensorValue2;
  // float currentRL2 = (rawValueRL2 - zeroCurrentADCRL2) * slope;  
  // float powerRL2 = Voltage * abs(currentRL2);  // Tính công suất (W)

  

  // energyConsumedRL2 += powerRL2 * elapsedTime; // Cộng dồn điện năng tiêu thụ (Wh)
  // Serial.print("ADC Value RL2: ");
  // Serial.print(rawValueRL2);
  // Serial.print(" | CurrentRL2: ");
  // Serial.print(currentRL2, 3);
  // Serial.print(" A | PowerRL2: ");
  // Serial.print(powerRL2, 3);
  // Serial.print(" W | Energy: ");
  // Serial.print(energyConsumedRL2, 3);
  // Serial.println(" Wh");

  delay(1000);
}
// void intitreadcapacity_Relay(){
//   RunningStatistics inputStats;                 // tạo đối tượng để đo lường
//   inputStats.setWindowSecs( windowLength );
//   lastUpdateTime = millis();
// }
// void readcapacity_Relay(const char* quserid){
//   // RunningStatistics inputStats;                 // tạo đối tượng để đo lường
//   // inputStats.setWindowSecs( windowLength );

//   sensorValue1 = analogRead(capability_RL1);  //đọc giá trị
//   sensorValue2 = analogRead(capability_RL2);  //đọc giá trị
//   sensorValue3 = analogRead(capability_RL3);  //đọc giá trị
//   sensorValue4 = analogRead(capability_RL4);  //đọc giá trị
//   sensorValue5 = analogRead(capability_RL5);  //đọc giá trị
//   sensorValue6 = analogRead(capability_RL6);  //đọc giá trị




  // float voltage = sensorValue2*3.3/1023.0;
  // #define SENSITIVITY 0.100 // Độ nhạy của ACS712 20A (100 mV/A)
  // #define VREF 3.3       // Điện áp tham chiếu của ESP32-S3
  // #define ADC_RESOLUTION 4095.0  // Độ phân giải 12-bit của ADC trên ESP32-S3
  // #define ZERO_CURRENT_VOLTAGE 1.71  // Điện áp khi dòng 0A (trung điểm của VCC)
  // float voltage = sensorValue2*VREF/ADC_RESOLUTION;
  // Serial.print("voltage: ");
  // Serial.println(voltage);
  // float current_test = -((voltage-ZERO_CURRENT_VOLTAGE)/SENSITIVITY)-1.19; // 0.185V/A for 5A max type ; 0.100V/A for 20A max type ; 0.66V/A for 30A max type
  // if (current_test < 0.02){
  //   current_test = 0;
  // }
  // Serial.print("current_test: ");
  // Serial.println(current_test);
  

  // inputStats.input(sensorValue2);  // đưa nó vào bộ kiểm tra
  // if ((millis() - previousMillis) >= printPeriod) {
  //   unsigned long currentTime = millis();
  //   float deltaT = (currentTime - lastUpdateTime) / 3600000.0; // Chuyển đổi ms → giờ
  //   lastUpdateTime = currentTime;

  //   previousMillis = millis();  // Cập nhật thời điểm in

  //   float rms = inputStats.sigma(); // Giá trị RMS của tín hiệu
  //   current_amps = rms * slope + intercept; // Chuyển đổi sang Ampere

  //   power = voltage * current_amps * 0.98;  // Công suất (W)
  //   energyWh += power * deltaT;  // Cộng dồn năng lượng tiêu thụ (Wh)

  //   Serial.print("RMS Value: ");
  //   Serial.print(rms);
  //   Serial.print(" | Current (A): ");
  //   Serial.print(current_amps);
  //   Serial.print(" | Power (W): ");
  //   Serial.print(power);
  //   Serial.print(" | Energy (Wh): ");
  //   Serial.println(energyWh);
  // }

  // int sum = 0;
  // int numSamples = 100;

  // for (int i = 0; i < numSamples; i++) {
  //     sum += sensorValue2;
  //     delay(10);
  // }

  // float zeroCurrentADC = sum / (float)numSamples;
  // Serial.print("Zero Current ADC: ");
  // Serial.println(zeroCurrentADC);
  // float zeroCurrentADC = 2119.00;
  // int rawValue = sensorValue2;
  // float current2 = (rawValue - zeroCurrentADC) * slope;  

  // Serial.print("ADC Value: ");
  // Serial.print(rawValue);
  // Serial.print(" | Current: ");
  // Serial.print(current2, 3);
  // Serial.println(" A");
  // float power = VOLTAGE * abs(current2);  // Tính công suất (W)
  // Serial.print("ADC Value: ");
  // Serial.print(rawValue);
  // Serial.print(" | Current: ");
  // Serial.print(current, 3);
  // Serial.print(" A | Power: ");
  // Serial.print(power, 3);
  // Serial.println(" W");

  // RunningStatistics inputStats;  // Tạo bộ lọc
  // inputStats.setWindowSecs(windowLength);

  // unsigned long startMillis = millis();
  // while (millis() - startMillis < windowLength * 1000) {
  //     inputStats.input(sensorValue2);
  // }

  // if ((millis() - previousMillis) >= printPeriod) {
  //     previousMillis = millis();

  //     // Chuyển đổi giá trị ADC thành Ampe
  //     float sigmaValue = inputStats.sigma();  // Giá trị RMS
  //     current_amps = intercept + slope * sigmaValue; 

  //     Serial.print("ADC RMS: ");
  //     Serial.print(sigmaValue);
  //     Serial.print(" | Current: ");
  //     Serial.print(current_amps, 3);
  //     Serial.println(" A");
  // }

//   Serial.println( "capacity_Relay1: " + String(sensorValue1) );
//   Serial.println( "capacity_Relay2: " + String(sensorValue2) );
//   Serial.println( "capacity_Relay3: " + String(sensorValue3) );
//   Serial.println( "capacity_Relay4: " + String(sensorValue4) );
//   Serial.println( "capacity_Relay5: " + String(sensorValue5) );
//   Serial.println( "capacity_Relay6: " + String(sensorValue6) );
  
// }
void TurnLight(const char* quserid)
{
    if (Firebase.ready())
      {
        // for (int led_gpio_pin = 25; led_gpio_pin <= 27; led_gpio_pin++)
        // {
          for (int i = 0; i <= 5; i++)
          { 
              led_gpio = i;
              switch (i){
                case 0: led_gpio_pin = 10; break;
                case 1: led_gpio_pin = 11; break;
                case 2: led_gpio_pin = 12; break;
                case 3: led_gpio_pin = 13; break;
                case 4: led_gpio_pin = 14; break;
                case 5: led_gpio_pin = 21; break;
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
                  // digitalWrite(led_gpio_pin, led_gpio == 0 ? LOW : HIGH);
                  digitalWrite(led_gpio_pin, LOW);
              }
              else if (led_gpio == i && led_state == "true") {
                  Serial.println(String("ESP32-GPIO ") + led_gpio_pin + String(" is ON"));
                  lcd.setCursor(0, i+1);  // display position
                  lcd.print(String(name_device) + ":"+ String(" ON"));
                  // digitalWrite(led_gpio_pin, led_gpio == 0 ? HIGH : LOW);
                  digitalWrite(led_gpio_pin, HIGH);
              }
              else {
                  Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, urlDevice) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
              }
          // }
        }
      
    }
}