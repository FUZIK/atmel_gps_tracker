#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define SEND_LOGS_VIA_WIFI_INTERVAL 10000

#define ESP_TX 2
#define ESP_RX 3

#define IO_TIMEOUT 100

#define SERVER_HOST "google.com"
#define API_PATH "/post_file"
#define SECRET_SSID "Andrew"
#define SECRET_PASS 88888888

#define WRITE_LOG_FILE "DATA.txt"

#define UPLOAD_BUFFER_SIZE 128

SoftwareSerial espSerial(ESP_TX, ESP_RX);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Serial begin on 9600");

  espSerial.begin(115200);
}

void postAllLog(File file) {
  // Подключение к точке
  espSerial.print("AT+CWMODE=3");
  String temp = "AT+CWJAP=\"";
  temp += SECRET_SSID;
  temp += "\",\"";
  temp += SECRET_PASS;
  temp += "\"";
  espSerial.print(temp);
  temp = "";
  delay(10000);
  // Вывод IP адресса модуля
  espSerial.print("AT+CIFSR");

  temp = "AT+CIPSTART=\"TCP\",\"";
  temp += SERVER_HOST;
  temp += "\",80";

  // Подключение к серверу
  espSerial.println(temp);

  // Отправка multipart запроса
  espSerial.print("POST ");
  espSerial.print(API_PATH);
  espSerial.print(" HTTP/1.1\r\n");

  espSerial.print("HOST: ");
  espSerial.print(SERVER_HOST);
  espSerial.print("\r\n");

  espSerial.print("User-Agent: esp8266_multipart/1.0\r\n");
  espSerial.print("Accept: */*\r\n");
  espSerial.print("Content-Type: multipart/form-data; boundary=X-ESP8266_MULTIPART\r\n");
  
  String bodyStart = "--X-ESP8266_MULTIPART\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"upfile\"; filename=\"data.txt\"\r\n";
  bodyStart += "Content-Type: application/octet-stream\r\n\r\n";

  String bodyEnd = "\r\n--X-ESP8266_MULTIPART--\r\n\r\n";

  int dataLenght = bodyStart.length() + bodyEnd.length() + file.size();

  espSerial.print("Content-Length: ");
  espSerial.print(dataLenght);
  espSerial.print("\r\n\r\n");

  espSerial.print(bodyStart);

  // Чтение и отправка файла на сервер  
  int tempPos = file.position();
  file.seek(0, SeekSet);
  uint8_t dataBuffer[128];
  while(file.available()) {
    while(file.position() < file.size()) {
      size_t len = file.readBytes( (char *)dataBuffer, sizeof(dataBuffer) );
      espSerial.write(dataBuffer, len);
    }
  }

  espSerial.print(bodyEnd);
  espSerial.flush();
}

unsigned long lastUploadMillis;

void loop() {
  long millisDiff = millis() - lastUploadMillis;

  if (millisDiff >= SEND_LOGS_VIA_WIFI_INTERVAL) {
    // postAllLog();
  }
}
