#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "testserver";
const char* password = "password";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // http endpoint where connection upgrade request should hit
AsyncWebSocketClient *Client;

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
  uint8_t data[] = {0x8a, 0x7e};
  Serial.println(sizeof(data));
  int length = sizeof(data);
  const char *message = "this is a message from ws server";
  int message_length = strlen(message);
  if(Client){
    sendBinaryData(Client, data, length);
    sendTextData(Client, message, message_length);
  }
  delay(7000);
}

void handleWebSocketMessage(void *arg, AsyncWebSocketClient *client, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    Serial.println("Text data received");
    data[len] = 0;
    char *data_string = (char*)data;
    Serial.printf("received text data = %s\n", data_string);
    // assign text data to variables
  }
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY) {
    Serial.println("binary data received");
    Serial.print("Binaray data length- ");
    Serial.println(len);
    Serial.print("Binary data- ");
    for(int i=0; i<len; i++){
      Serial.print(data[i], HEX);
    }
    // assign binary data to variables
  }
}

void sendBinaryData(AsyncWebSocketClient *client, uint8_t *data, int length){  // use sizeof to find length
  client->binary(data, length);
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