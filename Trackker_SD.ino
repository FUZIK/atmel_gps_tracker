#include <SPI.h>
#include <SD.h>
#include <TroykaGPS.h>

#define GPS_READ_INTERVAL 1000

#define GPS_TX A4 
#define GPS_RX A5
#define SD_CS 4

#define IO_TIMEOUT 100

#define WRITE_LOG_FILE "DATA.csv"

#define UPLOAD_BUFFER_SIZE 128

#define GPS_SERIAL Serial

GPS gps(GPS_SERIAL);

char time[16];

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial begin on 115200");

  Serial.println("Initializing SD card...");
  if (SD.begin(SD_CS)) {
    Serial.println("SD initialization done.");
  } else {
    Serial.println("Initialization failed.");
  }

  GPS_SERIAL.begin(115200);
  Serial.println("GPS init is OK on speed 115200");
}

void writeLog(char time[], float  speed, float  latitude, float  longitude) {
  File file = SD.open(WRITE_LOG_FILE, FILE_WRITE);
  if (file) {
    file.print("\"");
    file.print(time);
    file.print("\"");
    file.print(",");
    file.print(speed);
    file.print(",");
    file.print("\"");
    file.print(latitude, 6);
    file.print(",");
    file.print("\"");
    file.print(longitude, 6);
    file.println();
    file.close();
  }
}

unsigned long lastUploadMillis;

void loop() {
  long millisDiff = millis() - lastUploadMillis;

  if (millisDiff >= GPS_READ_INTERVAL) {
    if (gps.available()) {
      gps.readParsing();
      switch (gps.getState()) {
        case GPS_OK:
          gps.getTime(time, 16);
          writeLog(time, gps.getSpeedKm(), gps.getLatitudeBase10(), gps.getLongitudeBase10()); 
          break;
        case GPS_ERROR_DATA:
          Serial.println("GPS error data");
          break;
        case GPS_ERROR_SAT:
          Serial.println("GPS is not connected to satellites!!!");
          break;
      }
    }
  }
}
