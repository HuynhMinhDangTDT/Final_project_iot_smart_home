#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <Wire.h>

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
    int power;
};
Device devices[6];

// Cập nhật vị trí các nút sao cho cách đều và có khoảng cách ngăn giữa các nút và giữa các hàng
void updateButtonPositions() {
    int gap = 15; // Khoảng cách ngăn giữa các nút
    int rowGap = 20; // Khoảng cách giữa các hàng
    int buttonWidth = (screenWidth - 80 - 2 * gap) / 3;  // 3 cột, khoảng cách đều (giảm 40px cho khoảng cách cạnh và khoảng cách giữa các nút)
    int buttonHeight = (screenHeight - 160 - rowGap) / 2; // 2 hàng, khoảng cách đều, trừ 60px cho nhiệt độ và độ ẩm

    // Sắp xếp các nút trong 2 hàng và 3 cột, có khoảng cách giữa các hàng
    devices[0] = {40, 60, buttonWidth, buttonHeight, false, 0, 50}; 
    devices[1] = {40 + buttonWidth + gap, 60, buttonWidth, buttonHeight, false, 0, 75};
    devices[2] = {40 + 2 * (buttonWidth + gap), 60, buttonWidth, buttonHeight, false, 0, 100}; 
    
    devices[3] = {40, 130 + rowGap, buttonWidth, buttonHeight, false, 0, 150}; 
    devices[4] = {40 + buttonWidth + gap, 130 + rowGap, buttonWidth, buttonHeight, false, 0, 200}; 
    devices[5] = {40 + 2 * (buttonWidth + gap), 130 + rowGap, buttonWidth, buttonHeight, false, 0, 250}; 
}

void setup() {
    Serial.begin(115200);
    Wire.begin();  // Khởi tạo I2C Master
    tft.begin(tft.readID());
    tft.setRotation(1);  // Xoay màn hình ngang
    screenWidth = tft.width();
    screenHeight = tft.height();

    updateButtonPositions(); // Tính toán lại vị trí các nút
    drawUI();
}

void loop() {
    readTouch();
    updateStatus();
    delay(50);
}

// Vẽ giao diện ban đầu
void drawUI() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    
    for (int i = 0; i < 6; i++) {
        drawButton(i);
    }
    
    tft.setCursor(50, 10);
    tft.print("Smart Home");
}

// Vẽ từng nút bấm
void drawButton(int index) {
    tft.fillRect(devices[index].x, devices[index].y, devices[index].w, devices[index].h, 
                 devices[index].isOn ? GREEN : RED);
    tft.setCursor(devices[index].x + 20, devices[index].y + 15);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print(devices[index].isOn ? "ON" : "OFF");
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
        Serial.print("correctedTouchX: "); Serial.print(correctedTouchX);
        Serial.print(" | correctedTouchY: "); Serial.println(correctedTouchY);
        Serial.print("device[2]: ");Serial.print(devices[2].x);Serial.print(" | ");Serial.println(devices[2].y);
        Serial.print("device[5]: ");Serial.print(devices[5].x);Serial.print(" | ");Serial.println(devices[5].y);
        // Kiểm tra các nút
        for (int i = 0; i < 6; i++) {
            if (correctedTouchX >= devices[i].x - 70 && correctedTouchX <= devices[i].x - 70 + devices[i].w &&
                correctedTouchY >= devices[i].y + 30 && correctedTouchY <= devices[i].y + devices[i].h + 30 ){
                devices[i].isOn = !devices[i].isOn;
                if (devices[i].isOn) {
                    devices[i].startTime = millis();
                }
                drawButton(i);
                delay(50);
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
    // Giả sử lấy nhiệt độ và độ ẩm
    float temp = 30;
    float hum = 40;

    // Hiển thị nhiệt độ và độ ẩm
    tft.fillRect(40, screenHeight - 60, screenWidth - 80, 40, BLACK); // Chỉnh sửa vị trí
    tft.setCursor(40, screenHeight - 60);
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.print("Temp: "); tft.print(temp); tft.print(" C");
    tft.setCursor(40, screenHeight - 40);
    tft.print("Hum: "); tft.print(hum); tft.print(" %");

    // Hiển thị thông tin công suất và thời gian của các thiết bị đang bật
    for (int i = 0; i < 6; i++) {
        if (devices[i].isOn) {
            unsigned long elapsedTime = millis() - devices[i].startTime;
            unsigned long seconds = elapsedTime / 1000;
            unsigned long minutes = seconds / 60;
            unsigned long hours = minutes / 60;
            seconds = seconds % 60;

            // Chỉ cập nhật phần thay đổi
            tft.fillRect(devices[i].x, devices[i].y + 55, 100, 30, BLACK);
            tft.setCursor(devices[i].x + 10, devices[i].y + 55);
            tft.setTextColor(WHITE);
            tft.setTextSize(1);
            tft.print("Power: "); tft.print(devices[i].power); tft.print("W");

            // Hiển thị thời gian
            tft.setCursor(devices[i].x + 10, devices[i].y + 70);
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
