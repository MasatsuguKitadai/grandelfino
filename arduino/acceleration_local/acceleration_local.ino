#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define a 0.85
float rax, ray, raz, rgx, rgy, rgz;
String filename = ""; // To store the generated filename

const int chipSelect = 4;

unsigned long previousMillis = 0; // last time data was saved
const long interval = 20;         // interval at which to save data (milliseconds)

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // initialize the SD card
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect))
    {
        Serial.println("Card failed, or not present");
        while (1)
            ;
    }
    Serial.println("card initialized.");

    // initialize the MPU-6050
    Serial.println("Initializing MPU6050...");
    Wire.begin();
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();
    rax = ray = raz = rgx = rgy = rgz = 0;

    generateFilename();
    createFile(filename);
}

void generateFilename()
{
    int fileNumber = 1; // To increment the filename
    do
    {
        filename = String(fileNumber) + ".CSV";
        fileNumber++;
    } while (SD.exists(filename));
}

void createFile(String &filename)
{
    // ファイルを開く (存在しない場合は新規作成)
    File newFile = SD.open(filename, FILE_WRITE | O_TRUNC | O_CREAT); // ファイルを新規書き込みモードで開く

    // ファイルが正常に開かれたか確認
    if (newFile)
    {
        Serial.print("File created successfully.\t");
        Serial.println(filename);
        newFile.close();
    }
    else
    {
        Serial.println("Error creating file.");
    }
}

void loop()
{
    unsigned long currentMillis = millis();

    // Read from MPU-6050...
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);

    int16_t ax, ay, az, gx, gy, gz;
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
    Wire.read();
    Wire.read(); // Skip temperature data
    gx = Wire.read() << 8 | Wire.read();
    gy = Wire.read() << 8 | Wire.read();
    gz = Wire.read() << 8 | Wire.read();

    rax = a * rax + (1 - a) * ax / 16384.0;
    ray = a * ray + (1 - a) * ay / 16384.0;
    raz = a * raz + (1 - a) * az / 16384.0;
    rgx = a * rgx + (1 - a) * gx / 131.0;
    rgy = a * rgy + (1 - a) * gy / 131.0;
    rgz = a * rgz + (1 - a) * gz / 131.0;

    // Save to SD card every 'interval' milliseconds
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        File dataFile = SD.open(filename, FILE_WRITE);
        if (dataFile)
        {
            // Write to Serial monitor
            Serial.print("successfully opened " + filename + "\t");
            Serial.print(" time:");
            Serial.print(previousMillis);
            Serial.print(" rax:");
            Serial.print(rax);
            Serial.print(" ray:");
            Serial.print(ray);
            Serial.print(" raz:");
            Serial.print(raz);
            Serial.print(" rgx:");
            Serial.print(rgx);
            Serial.print(" rgy:");
            Serial.print(rgy);
            Serial.print(" rgz:");
            Serial.println(rgz);

            // Write to SD card
            dataFile.print(previousMillis);
            dataFile.print(",");
            dataFile.print(rax);
            dataFile.print(",");
            dataFile.print(ray);
            dataFile.print(",");
            dataFile.print(raz);
            dataFile.print(",");
            dataFile.print(rgx);
            dataFile.print(",");
            dataFile.print(rgy);
            dataFile.print(",");
            dataFile.println(rgz);

            dataFile.close();
        }
        else
        {
            Serial.print("error opening " + filename + "\t");
            Serial.print(" time:");
            Serial.print(previousMillis);
            Serial.print(" rax:");
            Serial.print(rax);
            Serial.print(" ray:");
            Serial.print(ray);
            Serial.print(" raz:");
            Serial.print(raz);
            Serial.print(" rgx:");
            Serial.print(rgx);
            Serial.print(" rgy:");
            Serial.print(rgy);
            Serial.print(" rgz:");
            Serial.println(rgz);
        }
    }
}

dataFile.print(previousMillis);
dataFile.print(",");
dataFile.print(rax);
dataFile.print(",");
dataFile.print(ray);
dataFile.print(",");
dataFile.print(raz);
dataFile.print(",");
dataFile.print(rgx);
dataFile.print(",");
dataFile.print(rgy);
dataFile.print(",");
dataFile.println(rgz);