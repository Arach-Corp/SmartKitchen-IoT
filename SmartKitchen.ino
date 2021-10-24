// Imports 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <HTTPClient.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <SPI.h>
#include <MFRC522.h>

// consts declare
#define SS_PIN 10
#define RST_PIN 9
#define HTTP_PORT 80
#define LED_VERMELHO 13
#define LED_VERDE 12

MFRC522 mfrc522(SS_PIN, RST_PIN);   // instance of MDRC522

String HTTP_METHOD = "GET";
String HOST_NAME = "localhost:1880/register"; // node-red url
String DEVICE_KEY = "123456789"; // key do dispositivo

char st[20];
 
void setup() 
{
  Serial.begin(115200);   // initialize a serial
  WiFiManager wifiManager;
  wifiManager.autoConnect("SmartKitchen"); 
  SPI.begin();      // starts spi
  mfrc522.PCD_Init();   // starts MFRC522
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
}
 
void loop() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  // read and converting of value in rfid
  String rfidCode="";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    rfidCode.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    rfidCode.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  rfidCode.toUpperCase();
  String productCode = String(rfidCode, DEC);

  // node-red http request
  bool successResult = getResponse(DEVICE_KEY, productCode);
  
  // turn on leds
  if (successResult) {
    digitalWrite(LED_VERDE, HIGH);
  } else {
    digitalWrite(LED_VERMELHO, HIGH);
  }

  delay(500);
  ledsOff();
} 
 
// turn off leds
void ledsOff() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
}

// http request
bool getResponse(String deviceKey, String productCode) {
  HTTPClient http;
  String serverUrl = HOST_NAME + "?dispositivoKey=" + deviceKey + "&loteId=" + productCode;
  
  http.begin(serverUrl.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    if (response.equals("OK")) {
      Serial.println("Success register");
      http.end();
      return true;
    } else {
      Serial.println("Failed register");
      http.end();
      return false;
    }
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}
