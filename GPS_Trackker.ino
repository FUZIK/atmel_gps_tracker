/* SD connect
 
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13
** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

*/

/* TroykaGPS connect
  ** TX 9
  ** RX 10
*/

/* ESP8266 connect
  ** CH_PD 3.3v
  ** RX 8
  ** TX 7
*/

#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <TroykaGPS.h>
#include <WiFiEspAT.h>

#define GPS_READ_INTERVAL 1000
#define SEND_LOGS_VIA_WIFI_INTERVAL 10000

#define GPS_TX 9 
#define GPS_RX 10
#define ESP_TX 8
#define ESP_RX 7
#define ESP_CH 4 

#define SECRET_SSID "Andrew"
#define SECRET_PASS 88888888

SoftwareSerial gpsSerial(GPS_TX, GPS_RX);

GPS gps(gpsSerial);
char gpsTime[16];

SoftwareSerial espSerial(ESP_TX, ESP_RX);

bool isUploadToWifiEnabled;


void setup() {
  Serial.begin(9600);
  while(!Serial);

  // esp8266
  espSerial.begin(9600); 
  WiFi.init(espSerial);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while(1);
  } else {
    Serial.println("WiFi module init!");
    WiFi.setPersistent();
  }

  // SD
  Serial.println("Initializing SD card...");
  if (SD.begin(ESP_CH)) {
    Serial.println("initialization done.");
    if (SD.exists("to_wifi_send.txt")) {
      mergeGpsLogToWifiLog();
      sendGpsLogsToServer();
    }
    File myFile = SD.open("gps_log.txt", FILE_WRITE);
    myFile.close();
  } else {
    Serial.println("Initialization failed. Retry");
    while(1);
  }

  // gps
  gpsSerial.begin(115200);
  Serial.println("GPS init is OK on speed 115200");
  gpsSerial.write("$PMTK251,9600*17\r\n");
  gpsSerial.end();
  gpsSerial.begin(9600);
  Serial.println("GPS init is OK on speed 9600");
}

void mergeGpsLogToWifiLog() {

  File toWifiSendFile = SD.open("to_wifi_send.txt", FILE_WRITE);
  File myFile = SD.open("gps_log.txt", FILE_WRITE);
  if (toWifiSendFile) {
    Serial.println("Wifi send file created.");
    while (myFile.available()) {
      toWifiSendFile.write(myFile.read());
    }
    toWifiSendFile.flush();
    toWifiSendFile.close();
    myFile.close();
  } else {
    Serial.println("Error while creating wifi send file. Retry");
    delay(1000);
  }
}

void writeToGpsLog(char time[], float  speed, float  latitude, float  longitude) {
  Serial.println("Write log to SD card");
  File myFile = SD.open("gps_log.txt", FILE_WRITE);
  myFile.println();
  myFile.print(time);
  myFile.print(speed);
  myFile.print(latitude);
  myFile.write(longitude);
  myFile.close();
}

void sendGpsLogsToServer() {
  WiFiClient client;
  isUploadToWifiEnabled = false;
  int status = WiFi.begin(SECRET_SSID, SECRET_PASS);
  if (status == WL_CONNECTED) {
    isUploadToWifiEnabled = false;
    Serial.println();
    Serial.println("Connected to WiFi network.");
    Serial.println("Starting connection to server...");
    if (client.connect("arduino.tips", 80)) {
      Serial.println("connected to server");
      Serial.println("Open file for sending to server");
      File toWifiSendFile = SD.open("to_wifi_send.txt", FILE_WRITE);
      if (toWifiSendFile) {
        Serial.println("Building request");
        client.println("POST / HTTP/1.1");
        client.print("Host: ");
        client.println("arduino.tips");
        client.println("Connection: close");
        client.println();
        Serial.println("Reading file");
        client.write(toWifiSendFile);
        client.flush();
        Serial.println("Request sended");
      }
    }
  } else {
    isUploadToWifiEnabled = true;
    WiFi.disconnect();
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }
}

unsigned long myTime;

void loop() {
  long myTimeDiff = millis() - myTime;

  if (myTimeDiff >= GPS_READ_INTERVAL) {
    if (gps.available()) {
      gps.readParsing();
      switch (gps.getState()) {
        case GPS_OK:
          gps.getTime(gpsTime, 16);
          writeToGpsLog(gpsTime, gps.getSpeedKm(), gps.getLatitudeBase10(), gps.getLongitudeBase10()); 
          break;
        case GPS_ERROR_DATA:
          Serial.println("GPS error data");
          break;
        case GPS_ERROR_SAT:
          Serial.println("GPS is not connected to satellites!!!");
          break;
      }
    } else {
      writeToGpsLog("TestTimeStr", 8.1234567f, 8.1234567f, 8.1234567f); 
    } 
  }

  if (isUploadToWifiEnabled && myTimeDiff >= SEND_LOGS_VIA_WIFI_INTERVAL) {
    sendGpsLogsToServer();
  }

  myTime = millis();
}
