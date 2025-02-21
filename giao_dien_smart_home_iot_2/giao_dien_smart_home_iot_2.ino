#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <Wire.h>
// #include <SoftwareSerial.h>  // Thư viện để sử dụng SoftwareSerial

// Chân RX và TX cho SoftwareSerial
// #define RX_PIN 16   // Chân RX (Arduino Mega)
// #define TX_PIN 17   // Chân TX (Arduino Mega)

// Khởi tạo SoftwareSerial với RX_PIN và TX_PIN
// SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX

// Cấu hình cảm ứng (tuỳ chỉnh theo màn hình)
#define YP A3  
#define XM A2  
#define YM 9  
#define XP 8
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define YELLOW  0xFFE0

#define MINPRESSURE 20
#define MAXPRESSURE 1000

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
int screenWidth, screenHeight;

// Thông tin trạng thái thiết bị
struct Device {
    int x, y, w, h;
    bool isOn;
    uint32_t startTime;
    float power;
    float energy;
    String name;
};
Device devices[6];

// Định nghĩa cấu trúc để lưu trữ dữ liệu
struct SensorData {
  float temp;   // Nhiệt độ
  float hum;    // Độ ẩm
  float energy; // Năng lượng
  float power;  // Công suất
};

// Khai báo biến toàn cục để lưu trữ dữ liệu
SensorData sensorData;

// Cập nhật vị trí các nút sao cho cách đều và có khoảng cách ngăn giữa các nút và giữa các hàng
void updateButtonPositions() {
    int gap = 15; // Khoảng cách ngăn giữa các nút
    int rowGap = 20; // Khoảng cách giữa các hàng
    int buttonWidth = (screenWidth - 80 - 2 * gap) / 3;  // 3 cột, khoảng cách đều (giảm 40px cho khoảng cách cạnh và khoảng cách giữa các nút)
    int buttonHeight = (screenHeight - 160 - rowGap) / 2; // 2 hàng, khoảng cách đều, trừ 60px cho nhiệt độ và độ ẩm

    // Sắp xếp các nút trong 2 hàng và 3 cột, có khoảng cách giữa các hàng
    devices[0] = {40, 60, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""}; 
    devices[1] = {40 + buttonWidth + gap, 60, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""};
    devices[2] = {40 + 2 * (buttonWidth + gap), 60, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""}; 
    
    devices[3] = {40, 130 + rowGap, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""}; 
    devices[4] = {40 + buttonWidth + gap, 130 + rowGap, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""}; 
    devices[5] = {40 + 2 * (buttonWidth + gap), 130 + rowGap, buttonWidth, buttonHeight, false, 0, 0.0, 0.0, ""}; 
}

void setup() {
    Serial.begin(115200);
    // mySerial.begin(9600); // Kết nối với ESP32-S3 qua SoftwareSerial
    Serial1.begin(115200);
    Wire.begin();  // Khởi tạo I2C Master
    tft.begin(tft.readID());
    tft.setRotation(1);  // Xoay màn hình ngang
    screenWidth = tft.width();
    screenHeight = tft.height();

    updateButtonPositions(); // Tính toán lại vị trí các nút
    drawUI();
}

// Nhận dữ liệu từ ESP32-S3 qua SoftwareSerial và cập nhật trạng thái
// void readFromESP32() {
//     if (mySerial.available() > 0) {
//         String data = mySerial.readString();  // Nhận chuỗi dữ liệu từ ESP32-S3
//         Serial.println(data);  // Hiển thị dữ liệu nhận được từ ESP32-S3
//         // Phân tích dữ liệu và cập nhật trạng thái thiết bị
//     }
// }


void updateDeviceState(String data) {
  // Tìm vị trí dấu phẩy (,) để tách ID thiết bị và trạng thái
  data.trim();
  if (data.startsWith("BUTTON:")) {
    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) return;  // Nếu không tìm thấy dấu phẩy, bỏ qua

    int deviceID = data.substring(7, commaIndex).toInt();  // Lấy ID thiết bị
    String state = data.substring(commaIndex + 1);  // Lấy trạng thái ("ON" hoặc "OFF")

    if (deviceID >= 0 && deviceID < 6) {  // Đảm bảo ID hợp lệ
        devices[deviceID].isOn = (state == "ON");  // Cập nhật trạng thái thiết bị
        drawButton(deviceID);  // Vẽ lại nút bấm với trạng thái mới
        // Serial.print("Updated Device ");
        // Serial.print(deviceID);
        // Serial.print(" to ");
        // Serial.println(state);
    }
  }
  else if (data.startsWith("ENERGY:")) {
    // String energyStr = data.substring(7);
    // float energy = energyStr.toFloat();

    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) return;  // Nếu không tìm thấy dấu phẩy, bỏ qua

    int deviceID = data.substring(7, commaIndex).toInt();  // Lấy ID thiết bị
    String energyStr = data.substring(commaIndex + 1);  // Lấy trạng thái ("ON" hoặc "OFF")

    float energy = energyStr.toFloat();
    // Serial.print("Updated Device ");
    // Serial.print(deviceID);
    // Serial.print("Energy: ");
    // Serial.println(energy);

    // for (int i = 0; i < 6; i++) {
    //     devices[i].energy = energy / 1000;
    // }
    devices[deviceID].energy = energy / 1000;
    
  }
  else if (data.startsWith("POWER:")) {
    // String powerStr = data.substring(6);
    // float power = powerStr.toFloat();
    // Serial.print("Power: "); Serial.println(power);
    // for (int i = 0; i < 6; i++) {
    //   devices[i].power = power;
    // }
    // devices[deviceID].power = power;
    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) return;  // Nếu không tìm thấy dấu phẩy, bỏ qua

    int deviceID = data.substring(6, commaIndex).toInt();  // Lấy ID thiết bị
    String powerStr = data.substring(commaIndex + 1);  // Lấy trạng thái ("ON" hoặc "OFF")

    float power = powerStr.toFloat();
    // Serial.print("Updated Device ");
    // Serial.print(deviceID);
    // Serial.print("Power: ");
    // Serial.println(power);

    // for (int i = 0; i < 6; i++) {
    //     devices[i].energy = energy / 1000;
    // }
    devices[deviceID].power = power;
  }
  else if (data.startsWith("NAME:")) {

    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) return;  // Nếu không tìm thấy dấu phẩy, bỏ qua

    int deviceID = data.substring(5, commaIndex).toInt();  // Lấy ID thiết bị
    String nameStr = data.substring(commaIndex + 1);  // Lấy trạng thái ("ON" hoặc "OFF")

    // Serial.print("Updated Device ");
    // Serial.print(deviceID);
    // Serial.print("Name: ");
    // Serial.println(nameStr);

    devices[deviceID].name = nameStr;
  }
  else if (data.startsWith("TEMP:")) {
    String tempStr = data.substring(5);
    float temp = tempStr.toFloat();
    // Serial.print("Temperature: "); Serial.println(temp);
    sensorData.temp = temp;
    // tft.setCursor(40, screenHeight - 60);
    // tft.setTextColor(YELLOW);
    // tft.setTextSize(2);
    // tft.fillRect(40, screenHeight - 60, screenWidth - 80, 20, BLACK);
    // tft.print("Temp: "); tft.print(temp); tft.print(" C");

  }
  else if (data.startsWith("HUM:")) {
    String humStr = data.substring(4);
    float hum = humStr.toFloat();
    sensorData.hum = hum;
    // Serial.print("Humidity: "); Serial.println(hum);
    // tft.setCursor(40, screenHeight - 40);
    // tft.setTextColor(YELLOW);
    // tft.setTextSize(2);
    // tft.fillRect(40, screenHeight - 40, screenWidth - 80, 20, BLACK);
    // tft.print("Hum: "); tft.print(hum); tft.print(" %");
  }
  else if (data.startsWith("TOTALENERGY:")) {
    String total_energyStr = data.substring(12);
    float total_energy = total_energyStr.toFloat();
    // Serial.print("Humidity: "); Serial.println(hum);
    sensorData.energy = total_energy / 1000;
    // tft.setCursor(300, screenHeight - 60);
    // tft.setTextColor(YELLOW);
    // tft.setTextSize(2);
    // tft.fillRect(300, screenHeight - 60, screenWidth - 80, 20, BLACK);
    // tft.print("TE: "); tft.print(total_energy / 1000); tft.print(" kWh");
  }
  else if (data.startsWith("TOTALPOWER:")) {
    String total_powerStr = data.substring(11);
    float total_power = total_powerStr.toFloat();
    // Serial.print("total_power: "); Serial.println(total_power);
    sensorData.power = total_power;
    // tft.setCursor(300, screenHeight - 40);
    // tft.setTextColor(YELLOW);
    // tft.setTextSize(2);
    // tft.fillRect(300, screenHeight - 40, screenWidth - 80, 20, BLACK);
    // tft.print("TP: "); tft.print(total_power); tft.print(" W");
  }
}

// void readFromESP32() {
//     if (Serial1.available()) {
//         String data = Serial1.readStringUntil(';');  // Đọc chuỗi đến khi gặp dấu ';'
//         data.trim();  // Loại bỏ khoảng trắng thừa
//         Serial.println("Received from ESP32-S3: " + data);
        
//         // Phân tích chuỗi nhận được
//         int idx1 = data.indexOf(',');
//         int idx2 = data.indexOf(',', idx1 + 1);
//         int idx3 = data.indexOf(',', idx2 + 1);
//         int idx4 = data.indexOf(',', idx3 + 1);
//         int idx5 = data.indexOf(',', idx4 + 1);
//         int idx6 = data.indexOf(',', idx5 + 1);

//         String btnID = data.substring(0, idx1);
//         String btnState = data.substring(idx1 + 1, idx2);
//         String btnName = data.substring(idx2 + 1, idx3);
//         String power = data.substring(idx3 + 1, idx4);
//         String energy = data.substring(idx4 + 1, idx5);
//         String temp = data.substring(idx5 + 1, idx6);
//         String humi = data.substring(idx6 + 1);

//         // Cập nhật giao diện hoặc xử lý thông tin
//         Serial.println("Button ID: " + btnID);
//         Serial.println("Button State: " + btnState);
//         Serial.println("Button Name: " + btnName);
//         Serial.println("Power: " + power);
//         Serial.println("Energy: " + energy);
//         Serial.println("Temperature: " + temp);
//         Serial.println("Humidity: " + humi);
//     }
// }

void loop() {
    readTouch();
    updateStatus();
    // readFromESP32();
    // Kiểm tra xem có dữ liệu từ ESP32-S3 gửi đến không
    if (Serial1.available()) {
        String data = Serial1.readStringUntil('\n');  // Đọc từng dòng dữ liệu
        data.trim();  // Loại bỏ khoảng trắng và ký tự xuống dòng thừa
        if (data.length() > 0) {
            Serial.print("Received: ");
            Serial.println(data);
            updateDeviceState(data);  // Gọi hàm cập nhật trạng thái thiết bị
        }
    }
    // delay(100);
}

// Vẽ giao diện ban đầu
void drawUI() {
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    
    for (int i = 0; i < 6; i++) {
        drawButton(i);
    }
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(50, 10);
    tft.print("Smart Home");
}

// Vẽ từng nút bấm
void drawButton(int index) {
    tft.fillRect(devices[index].x, devices[index].y, devices[index].w, devices[index].h, 
                 devices[index].isOn ? GREEN : RED);
    tft.setCursor(devices[index].x + 20, devices[index].y + 15);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.print(devices[index].isOn ? "ON" : "OFF");
    tft.setCursor(devices[index].x + 60, devices[index].y + 5);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.print(devices[index].name);
    // Serial.print("Sent: ID "); Serial.print(index);
    // Serial.print(" | State "); Serial.println(devices[index].isOn);
    // String data = String(index) + "," + (devices[index].isOn ? "ON" : "OFF");
    // Serial1.println(data);  // Gửi dữ liệu về trạng thái nút
}

// Xử lý cảm ứng
void readTouch() {
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    // Serial.print("Pressure: ");Serial.println(p.z);
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        // Ánh xạ tọa độ từ cảm ứng
        int touchX = map(p.x, 904, 170, 0, screenWidth);
        int touchY = map(p.y, 950, 158, 0, screenHeight);
        // int touchY = screenHeight - map(p.y, 950, 158, 0, screenHeight); // Đảo ngược giá trị Y
        // Đảo ngược tọa độ Y do màn hình đang ở chế độ landscape
        // Đảo ngược tọa độ do xoay màn hình ở chế độ landscape
        
        int correctedTouchX = touchY;                // Tọa độ Y của cảm ứng sẽ trở thành tọa độ X của màn hình
        int correctedTouchY = touchX;  // Tọa độ X của cảm ứng sẽ trở thành tọa độ Y của màn hình
        // Serial.print("correctedTouchX: "); Serial.print(correctedTouchX);
        // Serial.print(" | correctedTouchY: "); Serial.println(correctedTouchY);
        // Serial.print("device[2]: ");Serial.print(devices[2].x);Serial.print(" | ");Serial.println(devices[2].y);
        // Serial.print("device[5]: ");Serial.print(devices[5].x);Serial.print(" | ");Serial.println(devices[5].y);
        // Kiểm tra các nút
        for (int i = 0; i < 6; i++) {
            if (correctedTouchX >= devices[i].x - 70 && correctedTouchX <= devices[i].x - 70 + devices[i].w &&
                correctedTouchY >= devices[i].y + 100 && correctedTouchY <= devices[i].y + devices[i].h + 100 ){
                devices[i].isOn = !devices[i].isOn;
                if (devices[i].isOn) {
                    devices[i].startTime = millis();
                }
                drawButton(i);
                delay(50);
                // String data = String(i) + "," + (devices[i].isOn ? "ON" : "OFF");
                // mySerial.println(data);  // Gửi dữ liệu về trạng thái nút
                // Gửi dữ liệu trạng thái duy nhất một lần khi nhấn nút
                String data = String(i) + "," + (devices[i].isOn ? "ON" : "OFF");
                Serial1.println(data);
                Serial.print("Sent to ESP32-S3: ");
                Serial.println(data);
            }
        }

        // // Kiểm tra các nút
        // for (int i = 0; i < 6; i++) {
        //     if (touchX >= devices[i].x && touchX <= devices[i].x + devices[i].w &&
        //         touchY >= devices[i].y && touchY <= devices[i].y + devices[i].h) {
        //         devices[i].isOn = !devices[i].isOn;
        //         if (devices[i].isOn) {
        //             devices[i].startTime = millis();
        //         }
        //         drawButton(i);
        //         delay(300);
        //     }
        // }
    }
}

// Cập nhật trạng thái thiết bị và thông tin trên màn hình
void updateStatus() {
    // // Giả sử lấy nhiệt độ và độ ẩm
    // float temp = 30;
    // float hum = 40;

    // Hiển thị nhiệt độ và độ ẩm
    tft.fillRect(40, screenHeight - 60, screenWidth - 80, 40, BLACK); // Chỉnh sửa vị trí
    tft.setCursor(40, screenHeight - 60);
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.print("Temp: "); tft.print(sensorData.temp); tft.print(" C");
    tft.setCursor(40, screenHeight - 40);
    tft.print("Hum: "); tft.print(sensorData.hum); tft.print(" %");

    tft.setCursor(300, screenHeight - 60);
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.fillRect(300, screenHeight - 60, screenWidth - 80, 20, BLACK);
    tft.print("TE: "); tft.print(sensorData.energy); tft.print(" kWh");
  
    tft.setCursor(300, screenHeight - 40);
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.fillRect(300, screenHeight - 40, screenWidth - 80, 20, BLACK);
    tft.print("TP: "); tft.print(sensorData.power); tft.print(" W");

    // Hiển thị thông tin công suất và thời gian của các thiết bị đang bật
    for (int i = 0; i < 6; i++) {
        if (devices[i].isOn) {
            unsigned long elapsedTime = millis() - devices[i].startTime;
            unsigned long seconds = elapsedTime / 1000;
            unsigned long minutes = seconds / 60;
            unsigned long hours = minutes / 60;
            seconds = seconds % 60;

            // Chỉ cập nhật phần thay đổi
            tft.fillRect(devices[i].x, devices[i].y + 30, 100, 60, BLACK);
            tft.setCursor(devices[i].x + 10, devices[i].y + 30);
            tft.setTextColor(WHITE);
            tft.setTextSize(1);
            tft.print("Energy: "); tft.print(devices[i].energy); tft.print("kWh");

            tft.setCursor(devices[i].x + 10, devices[i].y + 45);
            tft.print("Power: "); tft.print(devices[i].power); tft.print("W");

            // Hiển thị thời gian
            tft.setCursor(devices[i].x + 10, devices[i].y + 55);
            tft.print("Time: ");
            if (hours < 10) tft.print("0");
            tft.print(hours);
            tft.print(":");
            if (minutes < 10) tft.print("0");
            tft.print(minutes);
            tft.print(":");
            if (seconds < 10) tft.print("0");
            tft.print(seconds);
        }
    }
}
