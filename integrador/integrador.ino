#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// =================== WiFi e IoT ===================
const char DEVICE_LOGIN_NAME[] = "f1b3463f-d598-450b-8fc4-719c09e7fa40";
const char DEVICE_KEY[] = "VPicx@LKIVVVxW5BrpGgy5yRs";
const char SSID[] = "Redmi Note 13";
const char PASS[] = "123cuatro5";
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// =================== Pines ===================
#define DHTPIN 23
#define DHTTYPE DHT11
#define SENSOR_TEMP 34
#define SENSOR_HUM 36
#define RELAY_BOMBA 15
#define RELAY_FAN 2
#define RELAY_FOCO 4
#define LED_WIFI 2  // LED interno del ESP32

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
AsyncWebServer server(80);

// =================== Variables Globales ===================
CloudSwitch bomba;
CloudSwitch fan;
CloudSwitch foco;
CloudTemperatureSensor temperatura;
CloudRelativeHumidity humedad;

float humedad_suelo = 0.0;
float humedad_aire = 0.0;
float temperatura_aire = 0.0;

int temperatura_base = 22;
int humedad_base = 20;

// =================== Keypad ===================
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12, 13};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// =================== IoT Callbacks ===================
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
  ArduinoCloud.addProperty(humedad, READ, ON_CHANGE, NULL);
}

void doThisOnConnect() {
  Serial.println("Conectado a Arduino IoT Cloud");
  digitalWrite(LED_WIFI, HIGH);
}

void doThisOnDisconnect() {
  Serial.println("Desconectado de Arduino IoT Cloud");
  digitalWrite(LED_WIFI, LOW);
}

// =================== Función teclado ===================
void teclado() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp Base:");
  String input = "";
  char key;

  while (true) {
    key = kpd.getKey();
    if (key) {
      if (key == '#') break;
      if (key >= '0' && key <= '9') {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(input);
      }
    }
  }
  temperatura_base = input.toInt();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hum Base:");
  input = "";

  while (true) {
    key = kpd.getKey();
    if (key) {
      if (key == '#') break;
      if (key >= '0' && key <= '9') {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(input);
      }
    }
  }
  humedad_base = input.toInt();
  lcd.clear();
}

// =================== Setup ===================
void setup() {
  Serial.begin(115200);
  delay(1500);

  // Pines
  pinMode(RELAY_BOMBA, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_FOCO, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(SENSOR_HUM, INPUT);
  pinMode(SENSOR_TEMP, INPUT);

  digitalWrite(RELAY_BOMBA, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_FOCO, LOW);
  digitalWrite(LED_WIFI, LOW);

  dht.begin();
  lcd.init();
  lcd.backlight();

  // WiFi
  WiFi.begin(SSID, PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Arduino IoT
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);

  // Servidor Web
  server.on("/datos", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("bomba")) bomba = request->getParam("bomba")->value() == "1";
    if (request->hasParam("fan")) fan = request->getParam("fan")->value() == "1";
    if (request->hasParam("foco")) foco = request->getParam("foco")->value() == "1";
    request->send(200, "text/plain", "Actuadores actualizados");
  });

  server.on("/sensores", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temp_aire\": " + String(temperatura_aire) + ",";
    json += "\"hum_aire\": " + String(humedad_aire) + ",";
    json += "\"hum_suelo\": " + String(humedad_suelo);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();

  lcd.setCursor(0, 0);
  lcd.print("Sistema Listo");
  delay(1000);
  lcd.clear();
}

// =================== Loop ===================
void loop() {
  ArduinoCloud.update();

  // =================== Lecturas ===================
  humedad_aire = dht.readHumidity();
  temperatura_aire = dht.readTemperature();

  int lectura_h = analogRead(SENSOR_HUM);
  humedad_suelo = 100 - (((lectura_h - 315) * 100) / 375); // Calibrar según sensor

  int lectura_t = analogRead(SENSOR_TEMP);
  float voltaje = lectura_t * (3.3 / 1023.0);
  float temp_suelo = (voltaje / 0.020) - 50;

  humedad = humedad_aire;
  temperatura = temperatura_aire;

  // =================== Mostrar en LCD ===================
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperatura_aire, 1);
  lcd.print("C H:");
  lcd.print(humedad_aire, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Hum Suelo:");
  lcd.print(humedad_suelo, 0);
  lcd.print("%");

  // =================== Teclado ===================
  char key = kpd.getKey();
  if (key == '*') {
    teclado();
  }

  // =================== Control automático ===================
  digitalWrite(RELAY_BOMBA, humedad_suelo < humedad_base ? HIGH : LOW);

  if (temperatura_aire < temperatura_base) {
    digitalWrite(RELAY_FOCO, HIGH);
    digitalWrite(RELAY_FAN, LOW);
  } else {
    digitalWrite(RELAY_FOCO, LOW);
    digitalWrite(RELAY_FAN, HIGH);
  }

  delay(500);
}
