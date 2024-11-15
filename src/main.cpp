#include <Arduino.h>
// network libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP32Servo.h>

// pin definitions
#define SERVO_PIN 26
#define POWER_PIN 27
#define CURRENT_PIN 32
#define VOLTAGE_PIN 33
#define CONNECT_PIN 34
#define CHARGE_PIN 22

// network
const char *ssid = "testserver";
const char* password = "password";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // http endpoint where connection upgrade request should hit
AsyncWebSocketClient *Client;

Servo steerServo;


// ws functions
void hanndleBinaryData(uint8_t *data, int size);
void handleTextData(uint8_t *data, int size);
void handleWebSocketMessage(void *arg, AsyncWebSocketClient *client, uint8_t *data, size_t len);
void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void sendBinaryData(AsyncWebSocketClient *client, uint8_t *data, int length);
void sendTextData(AsyncWebSocketClient *client, const char *message, int length);


void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("/");
  Serial.printf("Connected to %s with IP- ", ssid);
  Serial.println(WiFi.localIP());
  
  ws.onEvent(eventHandler);
  server.addHandler(&ws);
  server.begin();

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	steerServo.setPeriodHertz(50);
  steerServo.attach(SERVO_PIN, 500, 2400);  
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
}

void loop(){
  ws.cleanupClients();
  uint8_t binary_data[] = {0x7a, 0x99, 0x1c, 0xf3, 0x8b, 0x01, 0xbf};
  const char *text_data = "this is a ping from esp server";
  if(Client){
    ws.binaryAll(binary_data, sizeof(binary_data));
    ws.textAll(text_data);
  }
  for (int pos = 50; pos <= 140; pos += 1) { // goes from 0 degrees to 180 degrees
		// in steps of 1 degree
		steerServo.write(pos);    // tell servo to go to position in variable 'pos'
    Serial.println(pos);
		delay(15);             // waits 15ms for the servo to reach the position
	}
	for (int pos = 140; pos >= 50; pos -= 1) { // goes from 180 degrees to 0 degrees
		steerServo.write(pos);
    Serial.println(pos);    // tell servo to go to position in variable 'pos'
		delay(15);             // waits 15ms for the servo to reach the position
	}
  // steerServo.write(90);
  delay(100);
}

void handleWebSocketMessage(void *arg, AsyncWebSocketClient *client, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    handleTextData(data, len);
  }
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY) {
    hanndleBinaryData(data, len);
  }
}

void sendBinaryData(AsyncWebSocketClient *client, uint8_t *data, int length){  // use sizeof to find length
  client->binary(data, length);
}

void hanndleBinaryData(uint8_t *data, int size){
  Serial.print("received binary sequence- ");
  for(int i =0; i < size; i++){
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

void handleTextData(uint8_t *data, int size){
  char *message = (char*)data;
  Serial.print("received message- ");
  Serial.println(message);
}

void sendTextData(AsyncWebSocketClient *client, const char *message, int length){ // use strlen to find length
  client->text(message, length);
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      if(client->id() == 1){
        Serial.println("first client");
        Client = client; // make first connection the main client
      }
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, client, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}