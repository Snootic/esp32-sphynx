#include "SphynxWiFi.h"
#include "ESPAsyncWebServer.h"

#include <stdio.h>
#include <string.h>

#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 5
#define RST_PIN 4

#define button 21

#define led 2

MFRC522 rfid(SS_PIN, RST_PIN);

int acionador = 15;

String message;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
    ws.textAll("$2a$12$X9my8HHbMJYk6y04FnR6ie1B/WnLOlBAeEMRhEOvt.8z/OmOR6kLS");
  }      
   
  else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
  }

  else if(type == WS_EVT_DATA){
    handleWebSocketMessage(arg, data, len);
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    controlDoor(message);
  }
}
 
void controlDoor(String message){
  if(message == "true"){
    digitalWrite(led, !digitalRead(led));
    delay(1000);    
    digitalWrite(led, !digitalRead(led));
  }
  else if(message == "false"){
    digitalWrite(led, !digitalRead(led));
    delay(500);
    digitalWrite(led, !digitalRead(led));  
    delay(500);
    digitalWrite(led, !digitalRead(led));  
    delay(500);
    digitalWrite(led, !digitalRead(led));  
  }
}

void sphynx(){
  // Sphynx default operation
  Serial.println("Sphynx Begun");
  SPI.begin();
  rfid.PCD_Init();
  delay(2000);

  pinMode(acionador, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);

  Serial.println("Versão Firmware RC522");
  rfid.PCD_DumpVersionToSerial();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.on("/conectar", HTTP_GET, [](AsyncWebServerRequest * request) {
  request->send(200, "text/plain", "Clyio - Sphynx");
  });
}

void setup(){
  Serial.begin(115200);
  if (SphynxWiFi.connect() != 0) {
    SphynxWiFi.setupWiFi();
  }
  while (SphynxWiFi.status() != WL_CONNECTED){
    continue;
  }
  sphynx();
}

void loop(){
  // rfid.PCD_DumpVersionToSerial();
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
    Serial.println("Coloque o cartão no Leitor.");
    
    String id_cartao = "";
    byte i;
          
    for (byte i = 0; i < rfid.uid.size; i++){
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
      id_cartao.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
      id_cartao.concat(String(rfid.uid.uidByte[i], HEX));
    }
      
    id_cartao.toUpperCase();
    Serial.println(id_cartao.substring(1));
    
  }

  if(digitalRead(button) == 0){
    return;
  }else{
    ws.textAll("abc");
  }
  
  delay(500);
}
