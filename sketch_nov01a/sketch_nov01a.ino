#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

// Initialize pins
int redpin = D0;
int greenpin = D2;
int bluepin = D4;

// Setting up the websocket client
WebSocketsClient webSocket;

// Set up the WiFi client;
WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  pinMode(redpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);

  // Initialize WiFi Manager
  WiFiManager wifiManager;
  // Initialize access point network without password
  wifiManager.autoConnect("LightNode");
  
  // Initializing the WS2812B communication
  setRgb(255,80,90);

  // Initializing the websocket connection

  // When in development, replacement first argument in webSocket.begin() function with your machine's IP address for your Wi-Fi network. Make sure to run your open your local server up to other devices by running `rails s -b 0.0.0.0` so your arduino can connect in.

  // When in production, replace the below with your production URL and port. E.g. webSocket.begin("nodemcu-led-lights.herokuapp.com", 80, "/cable")
  webSocket.begin("192.168.1.93",3000, "/cable" );
//  webSocket.begin("arduino-actioncable-led.herokuapp.com",80, "/cable" );
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5);

}
void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();

} 

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    Serial.write(type);
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      webSocket.setExtraHeaders("Content-Type: application/json");
      // The Arduino is currently configured to join one channel. In future, if you would like to expand to include many times of smart-devices, you could expand your channels and dynamically include the channel name in the below string depending on the nature of your device. 
      webSocket.sendTXT("{\"command\":\"subscribe\",\"identifier\":\"{\\\"channel\\\":\\\"ArduinoChannel\\\"}\"}");
      webSocket.sendTXT("{\"command\":\"message\",\"data\":\"{\\\"action\\\": \\\"identify_device\\\", \\\"mac\\\":\\\"" + WiFi.macAddress() + "\\\"}\",\"identifier\":\"{\\\"channel\\\":\\\"ArduinoChannel\\\"}\"}");

      Serial.printf("[WSc] Connected to url: %s\n", payload);
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] Received text: %s\n", payload);
      DynamicJsonBuffer jBuffer;
      JsonObject &root = jBuffer.parseObject(payload);
      if (root["message"]["rgb"]){
        setRgb(root["message"]["rgb"]["r"],root["message"]["rgb"]["g"],root["message"]["rgb"]["b"]);
      }
      break;
  }
}

  void setRgb(uint8_t r, uint8_t g, uint8_t b) {
   Serial.print(r);
   Serial.print(g);
   Serial.print(b);
  analogWrite(redpin, r);
  analogWrite(bluepin, b);
  analogWrite(greenpin, g);
  delay(10);
}
