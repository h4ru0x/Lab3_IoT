#include <WiFi.h>
#include <PubSubClient.h>

// --- CONFIGURACIÓN WiFi ---
const char* ssid = "h4";
const char* password = "clasedeiot";

// --- CONFIGURACIÓN MQTT ---
const char* mqtt_broker = "10.140.65.133";
const int mqtt_port = 1883;
const char* topic = "lab/esp32/data";

// --- OBJETOS WiFi/MQTT ---
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);   // Para debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("WiFi conectado!");

  // Conectar al broker MQTT
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Conectando al broker como %s...\n", client_id.c_str());
    if (client.connect(client_id.c_str())) {
      Serial.println("Conectado al broker MQTT");
    } else {
      Serial.print("Fallo, estado: ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  // Publicación de prueba
  client.publish(topic, "ESP32 listo para recibir datos.");
}

void loop() {
  client.loop();

  // Leer datos del Arduino
  if (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    message.trim();

    if (message.length() > 0) {
      Serial.print("Mensaje recibido: ");
      Serial.println(message);

      // Publicar en MQTT
      client.publish(topic, message.c_str());
    }
  }
}
