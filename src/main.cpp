#include <Arduino.h>
// network libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// network
const char *ssid = "testserver";
const char* password = "password";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // http endpoint where connection upgrade request should hit
AsyncWebSocketClient *Client;

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
}

void loop(){
  ws.cleanupClients();
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