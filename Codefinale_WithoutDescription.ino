#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <Wire.h>
#include "DHT11.h"
#include "DHTesp.h"
#include "FS.h"
#include <HardwareSerial.h>

#define sck 18
#define miso 19
#define mosi 23
#define cs 5
#define DHTPIN 21       
#define DHTTYPE DHT11   
#define SD_CS 5         

DHT dht(DHTPIN, DHTTYPE);
HardwareSerial mySerial(2); 

byte getPPM[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

File dataFile;

void setup() {
  Serial.begin(115200);
  dht.begin();

  mySerial.begin(9600, SERIAL_8N1, 16, 17); 

   SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
    Serial.println("Card Mount Failed");
    while (1);
  }

  Serial.println("Initialization complete. Logging data...");
  
  if (!SD.exists("/Test.csv")) {
    Serial.println("Creating new data file...");
  }
}

int readCO2() {
  byte response[9];
  int checksum = 0;

  mySerial.write(getPPM, 9);
  delay(10);

  if (mySerial.available()) {
    for (int i = 0; i < 9; i++) {
      response[i] = mySerial.read();
    }
    if (response[0] == 0xFF && response[1] == 0x86) {
      checksum = 0;
      for (int i = 1; i < 8; i++) {
        checksum += response[i];
      }
      checksum = 0xFF - checksum;
      checksum++;

      if (response[8] == checksum) {
        int high = response[2];
        int low = response[3];
        int ppm = (high << 8) + low;
        return ppm;
      }
    }
  }
  return -1; 
}

void logData(float temperature, float humidity, int co2_ppm) {

  dataFile = SD.open("/Test.csv", FILE_APPEND);
  if (dataFile) {
    char charVal1[10];
    dtostrf(temperature, 4, 3, charVal1);
    appendFile(SD, "/Test.csv", charVal1);
    char ziel1[50] = ",  ";
    char quelle1[] = "   ";
    strcat(ziel1, quelle1);
    appendFile(SD, "/Test.csv", ziel1);
    
    char charVal2[10];
    dtostrf(humidity, 4, 3, charVal2);
    appendFile(SD, "/Test.csv", charVal2);
    char ziel2[50] = ",  ";
    char quelle2[] = "          ";
    strcat(ziel2, quelle2);
    appendFile(SD, "/Test.csv", ziel2);
    
    char charVal3[10];
    dtostrf(co2_ppm, 4, 3, charVal3);
    appendFile(SD, "/Test.csv", charVal3);
    char ziel3[50] = ",    ";
    char quelle3[] = "   ";
    strcat(ziel3, quelle3);
    appendFile(SD, "/Test.csv", ziel3);

    char charVal4[10];
    dtostrf(co2_ppm / 10000, 4, 3, charVal4);
    appendFile(SD, "/Test.csv", charVal4);
    char ziel4[50] = "   ";
    char quelle4[] = "\n ";
    strcat(ziel4, quelle4);
    appendFile(SD, "/Test.csv", ziel4);
    
    dataFile.close();
    Serial.println("Data logged.");
  } else {
    Serial.println("Failed to open data file.");
  }
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int co2_ppm = readCO2();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, CO2: ");
  Serial.print(co2_ppm);
  Serial.println(" ppm");

  logData(temperature, humidity, co2_ppm);

  delay(2000);
}
