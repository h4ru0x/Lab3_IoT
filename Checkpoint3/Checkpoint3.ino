#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SENSOR_HUMEDAD_PIN 34
#define POTENCIOMETRO_PIN 35
#define RELE_BOMBA_PIN 26
#define LED_ROJO_PIN 25
#define LED_VERDE_PIN 33
#define LED_AZUL_PIN 32

const char* ssid = "Iphone de Lenin";
const char* password = "arrozconpollo123";
const char* mqtt_server = "172.20.10.15";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
bool modo_manual = false;

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  if (String(topic) == "RIEGO/CONTROL") {
    if (mensaje == "ON") {
      modo_manual = true;
      digitalWrite(RELE_BOMBA_PIN, HIGH);
    } else if (mensaje == "OFF") {
      modo_manual = true;
      digitalWrite(RELE_BOMBA_PIN, LOW);
    } else if (mensaje == "AUTO") {
      modo_manual = false;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      client.subscribe("RIEGO/CONTROL");
      digitalWrite(LED_AZUL_PIN, HIGH);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      digitalWrite(LED_AZUL_PIN, LOW);
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELE_BOMBA_PIN, OUTPUT);
  pinMode(LED_ROJO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_AZUL_PIN, OUTPUT);
  
  digitalWrite(RELE_BOMBA_PIN, LOW);
  digitalWrite(LED_AZUL_PIN, LOW);
  digitalWrite(LED_ROJO_PIN, LOW);
  digitalWrite(LED_VERDE_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    digitalWrite(LED_AZUL_PIN, LOW);
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    int valorHumedad = analogRead(SENSOR_HUMEDAD_PIN);
    int valorUmbral = analogRead(POTENCIOMETRO_PIN);
    
    int humedadMapeada = map(valorHumedad, 0, 4095, 100, 0);
    int umbralMapeado = map(valorUmbral, 0, 4095, 0, 100);

    if (!modo_manual) {
      if (humedadMapeada < umbralMapeado) {
        digitalWrite(RELE_BOMBA_PIN, HIGH);
        digitalWrite(LED_ROJO_PIN, HIGH);
        digitalWrite(LED_VERDE_PIN, LOW);
      } else {
        digitalWrite(RELE_BOMBA_PIN, LOW);
        digitalWrite(LED_ROJO_PIN, LOW);
        digitalWrite(LED_VERDE_PIN, HIGH);
      }
    }

    String estadoBomba = (digitalRead(RELE_BOMBA_PIN) == HIGH) ? "ON" : "OFF";
    
    StaticJsonDocument<128> doc;
    doc["HUMEDAD"] = humedadMapeada;
    doc["UMBRAL"] = umbralMapeado;
    doc["ESTADO_BOMBA"] = estadoBomba;
    
    char buffer[128];
    serializeJson(doc, buffer);
    client.publish("RIEGO/DATOS", buffer);
  }
}