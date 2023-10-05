#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10; // SDカードモジュールのチップセレクトピン番号
const float alpha = 0.85;
const float interval = 20; // [ms]
float rax, ray, raz, rgx, rgy, rgz;

void setup() {
    Serial.begin(115200);
    Serial.println("--- Started ---");
    initializeSDCard();
    initializeMPU6050();
    String fileName = generateFileName();
    createNewFile(fileName);
}

void loop() {
    int startTime = millis();
    readMPU6050Data();

    static unsigned long previousTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - previousTime >= interval) {  // interval [ms] sampling interval
        writeToSD(generateFileName(), currentTime);
        previousTime = currentTime;
    }

    delayToMaintainLoopFrequency(startTime, interval); // interval [ms] loop time
}

void initializeSDCard() {
    if (!SD.begin(chipSelect)) {
        Serial.println("Failed to initialize SD card!");
        return;
    }
    Serial.println("SD card initialized successfully.");
}

void initializeMPU6050() {
    Wire.begin();
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();
    rax = ray = raz = rgx = rgy = rgz = 0;
}

void readMPU6050Data() {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);

    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();
    int16_t gx = Wire.read() << 8 | Wire.read();
    int16_t gy = Wire.read() << 8 | Wire.read();
    int16_t gz = Wire.read() << 8 | Wire.read();

    rax = alpha * rax + (1-alpha) * ax / 16384.0;
    ray = alpha * ray + (1-alpha) * ay / 16384.0;
    raz = alpha * raz + (1-alpha) * az / 16384.0;
    rgx = alpha * rgx + (1-alpha) * gx / 131.0;
    rgy = alpha * rgy + (1-alpha) * gy / 131.0;
    rgz = alpha * rgz + (1-alpha) * gz / 131.0;
}

String generateFileName() {
    char fileName[25]; // Enough to hold "YYYYMMDD_HHMMSS.csv"
    snprintf(fileName, sizeof(fileName), "%04d%02d%02d_%02d%02d%02d.csv", year(), month(), day(), hour(), minute(), second());
    return String(fileName);
}

void createNewFile(String fileName) {
    File Write_File = SD.open(fileName, FILE_WRITE | O_TRUNC | O_CREAT);
    if (Write_File) {
        Write_File.println("Time, rax, ray, raz, rgx, rgy, rgz");
        Write_File.close();
    } else {
        Serial.println("ファイルの作成に失敗しました！");
    }
}

void writeToSD(String fileName, unsigned long time) {
    File WriteFile = SD.open(fileName, FILE_WRITE);
    if (WriteFile) {
        WriteFile.print(time);
        WriteFile.print(",");
        WriteFile.print(rax);
        WriteFile.print(",");
        WriteFile.print(ray);
        WriteFile.print(",");
        WriteFile.print(raz);
        WriteFile.print(",");
        WriteFile.print(rgx);
        WriteFile.print(",");
        WriteFile.print(rgy);
        WriteFile.println(rgz);
        WriteFile.close();
    }
}

void delayToMaintainLoopFrequency(int startTime, int desiredLoopTime) {
    int elapsedTime = millis() - startTime;
    if (elapsedTime < desiredLoopTime) {
        delay(desiredLoopTime - elapsedTime);
    }
}
