#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <Keypad.h>

// ----- IoT Cloud -----
const char DEVICE_LOGIN_NAME[] = "f1b3463f-d598-450b-8fc4-719c09e7fa40";
const char DEVICE_KEY[]        = "VPicx@LKIVVVxW5BrpGgy5yRs";
const char SSID[]              = "Redmi Note 13";
const char PASS[]              = "123cuatro5";

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// ----- Pines y objetos -----
#define DHTPIN     4
#define DHTTYPE    DHT11
#define RELAY_BOMBA 23
#define RELAY_FAN   22
#define RELAY_FOCO  21
#define LED_WIFI     2

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

// ----- Variables Cloud -----
CloudSwitch bomba;
CloudSwitch fan;
CloudSwitch foco;
CloudTemperatureSensor temperatura;

// ----- Humedad -----
float humedad = 0.0;

// ----- Teclado Matricial 4x4 -----
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFilas[FILAS] = {14, 27, 26, 25}; // Ajusta si es necesario
byte pinesColumnas[COLUMNAS] = {33, 32, 35, 34};
Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// ------------------- FUNCIONES IoT -------------------
void onBombaChange() {
  digitalWrite(RELAY_BOMBA, bomba ? HIGH : LOW);
}
void onFanChange() {
  digitalWrite(RELAY_FAN, fan ? HIGH : LOW);
}
void onFocoChange() {
  digitalWrite(RELAY_FOCO, foco ? HIGH : LOW);
}
void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(bomba, READWRITE, ON_CHANGE, onBombaChange);
  ArduinoCloud.addProperty(fan, READWRITE, ON_CHANGE, onFanChange);
  ArduinoCloud.addProperty(foco, READWRITE, ON_CHANGE, onFocoChange);
  ArduinoCloud.addProperty(temperatura, READ, ON_CHANGE, NULL);
}

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_BOMBA, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_FOCO, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  digitalWrite(RELAY_BOMBA, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_FOCO, LOW);
  digitalWrite(LED_WIFI, LOW);

  dht.begin();

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, []() {
    digitalWrite(LED_WIFI, HIGH);
    Serial.println("Conectado a Arduino IoT Cloud");
  });
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, []() {
    digitalWrite(LED_WIFI, LOW);
    Serial.println("Desconectado de IoT Cloud");
  });

  // ----- Endpoint /datos -----
  server.on("/datos", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("bomba")) {
      bomba = request->getParam("bomba")->value() == "1";
      digitalWrite(RELAY_BOMBA, bomba ? HIGH : LOW);
    }
    if (request->hasParam("fan")) {
      fan = request->getParam("fan")->value() == "1";
      digitalWrite(RELAY_FAN, fan ? HIGH : LOW);
    }
    if (request->hasParam("foco")) {
      foco = request->getParam("foco")->value() == "1";
      digitalWrite(RELAY_FOCO, foco ? HIGH : LOW);
    }
    request->send(200, "text/plain", "Datos recibidos desde /datos");
  });

  // ----- Endpoint /sensores -----
  server.on("/sensores", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temperatura\": " + String(temperatura) + ",";
    json += "\"humedad\": " + String(humedad);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

  
void loop() {
  ArduinoCloud.update();

}
