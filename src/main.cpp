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
int servo_angle = 90;
bool power_state = false;
int servo_angle_c = 90;

float cap_voltage = 0;
float current_draw = 0;

unsigned long last_time = 0;

// ws functions
void hanndleBinaryData(uint8_t *data, int size);
void handleTextData(uint8_t *data, int size);
void handleWebSocketMessage(void *arg, AsyncWebSocketClient *client, uint8_t *data, size_t len);
void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void sendBinaryData(AsyncWebSocketClient *client, uint8_t *data, int length);
void sendTextData(AsyncWebSocketClient *client, const char *message, int length);

void setMotorSpeed(int pwm_val);
void setServoAngle(int angle);

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
  steerServo.write(servo_angle);
  analogSetWidth(12);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(CURRENT_PIN, INPUT);
}

void loop(){
  ws.cleanupClients();
  digitalWrite(POWER_PIN, power_state);
  steerServo.write(servo_angle_c);
  if(millis() - last_time > 1000){
    cap_voltage = float(analogRead(VOLTAGE_PIN))*(3.3/4095.0)*7.8;
    current_draw = (float(analogRead(CURRENT_PIN))*float(3.3/4095.0) - 2.5)*0.2 ;
    Serial.printf("Volatge- %f, Current- %f\n", cap_voltage, current_draw);
    uint8_t readings[] = {uint8_t(cap_voltage), uint8_t(current_draw)};
    int len = sizeof(readings);
    if(Client){
      sendBinaryData(Client, readings, len);
    }
    last_time = millis();
  }
  delay(10);
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
  if(data[0] == 0xFF){
    Serial.println("motor turned on");
    power_state = true;
  }
  else if(data[0] == 0x01){
    Serial.println("motor turned off");
    power_state = false;
  }
  if(data[1] == 0xA2){
    Serial.println("Steer left");
    servo_angle_c = 20;
  }
  else if(data[1] == 0xA0){
    servo_angle_c = 90;
  }
  else if(data[1] == 0xA1){
    Serial.println("Steer right");
    servo_angle_c = 150;
  }
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
