#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

IPAddress local_IP(192,168,4,20);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

const int dry = 500;
bool isautomatic = false;
bool isWatering = false;
int sensorValue;
int relayInput = D2;

void handleRoot() {
  int sensorValue = analogRead(A0);
  bool isDry = false;
  //check
  if(sensorValue >= dry){
    isDry = true;  
  }
  
  //parse json
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["success"] = true;
  root["automatic_watering"] = isautomatic;
  root["humidity"] = sensorValue;
  root["dry"] = isDry;

  String jsonMsg;
  root.prettyPrintTo(jsonMsg);
  
  server.send(200, "application/json", jsonMsg);
}

void handleOn() {
  isautomatic = true;

  //parse json
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["success"] = true;
  root["automatic_watering"] = isautomatic;
  

  String jsonMsg;
  root.prettyPrintTo(jsonMsg);

  digitalWrite(relayInput, HIGH);
  server.send(200, "application/json", jsonMsg);
}

void handleOff() {
  isautomatic = false;
  
  //parse json
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["success"] = true;
  root["automatic_watering"] = isautomatic;
  

  String jsonMsg;
  root.prettyPrintTo(jsonMsg);

  digitalWrite(relayInput, LOW);
  server.send(200, "application/json", jsonMsg);
}

void handleWatering() {
  digitalWrite(relayInput, HIGH);
  
  //parse json
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["success"] = true;
  

  String jsonMsg;
  root.prettyPrintTo(jsonMsg);
  
  server.send(200, "application/json", jsonMsg);
}

void handleNotFound() {
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  //parse json
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["success"] = false;
  root["automatic_watering"] = isautomatic;
  root["message"] = message;
  

  String jsonMsg;
  root.prettyPrintTo(jsonMsg);
  
  server.send(404, "application/json", jsonMsg);
}

void watering() {
  sensorValue = analogRead(A0);
  
  Serial.println(sensorValue);
  if (sensorValue >= dry){
    digitalWrite(relayInput, HIGH);
    isWatering = true;
  } else {
    digitalWrite(relayInput, LOW);
    isWatering = false;
  }
}

void setup(void) {
  pinMode(relayInput, OUTPUT);
  Serial.begin(115200);
  digitalWrite(relayInput, LOW);
  WiFi.mode(WIFI_AP);
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("Primakara Automatic Watering", "test1234") ? "Ready" : "Failed!");
  
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/flush", handleWatering);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();

  if (isautomatic) {
    watering();
  } else {
    sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    if (sensorValue < dry) {
      digitalWrite(relayInput, LOW);  
    }
  }

  delay(1500);
}
