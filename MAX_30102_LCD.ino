#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MAX30105.h>
#include "heartRate.h"

#define BUZZER_PIN 8            // Chân kết nối chuông
#define VIBRATION_PIN 9         // Chân kết nối module rung
#define NORMAL_MIN_HR 60        // Giới hạn nhịp tim thấp
#define NORMAL_MAX_HR 100       // Giới hạn nhịp tim cao
#define DISPLAY_TIME 5000       // Thời gian hiển thị kết quả nhịp tim (ms)

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Địa chỉ LCD có thể thay đổi tùy loại màn hình

float heartRate;       // Biến lưu nhịp tim
bool isWarning = false;
unsigned long displayStartTime = 0;  // Thời gian bắt đầu hiển thị kết quả
bool displayActive = false;          // Trạng thái hiển thị kết quả nhịp tim

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Heart Rate Monitor");

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(VIBRATION_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIBRATION_PIN, LOW);

    // Khởi tạo cảm biến MAX30105
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("Lỗi: Không tìm thấy cảm biến MAX30105!");
        lcd.setCursor(0, 1);
        lcd.print("Sensor Error!");
        while (true);
    }
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);    // Cường độ đèn đỏ cho nhịp tim
    particleSensor.setPulseAmplitudeGreen(0);     // Tắt đèn xanh vì không cần thiết
}

void loop() {
    long irValue = particleSensor.getIR();    // Lấy giá trị IR từ cảm biến

    if (checkForBeat(irValue)) {    // Phát hiện nhịp đập
        heartRate = 60 / (millis() / 1000.0); // Tính nhịp tim theo BPM
        heartRate = constrain(heartRate, 0, 250); // Giới hạn nhịp tim trong khoảng hợp lý
        Serial.print("HR: ");
        Serial.println(heartRate);
        
        // Hiển thị nhịp tim trên LCD và khởi động thời gian hiển thị
        lcd.setCursor(0, 1);
        lcd.print("HR: ");
        lcd.print(heartRate);
        lcd.print(" BPM   ");
        displayStartTime = millis();     // Ghi lại thời gian bắt đầu hiển thị
        displayActive = true;

        // Kiểm tra nếu nhịp tim vượt quá giới hạn bình thường
        if (heartRate < NORMAL_MIN_HR || heartRate > NORMAL_MAX_HR) {
            if (!isWarning) {
                digitalWrite(BUZZER_PIN, HIGH);
                digitalWrite(VIBRATION_PIN, HIGH);
                isWarning = true;
            }
        } else {
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(VIBRATION_PIN, LOW);
            isWarning = false;
        }
    }

    // Kiểm tra nếu đã hiển thị quá 5 giây, xóa màn hình và quay lại trạng thái chờ
    if (displayActive && (millis() - displayStartTime >= DISPLAY_TIME)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Heart Rate Monitor");
        displayActive = false;
    }

    delay(100);  // Giảm tần số đo để tránh dao động lớn
}
