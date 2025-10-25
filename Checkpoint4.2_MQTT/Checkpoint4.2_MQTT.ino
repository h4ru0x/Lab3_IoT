void setup() {
  Serial.begin(9600);   // Monitor Serial (para depuración)
  Serial1.begin(9600);  // Comunicación UART hacia el ESP32
}

void loop() {
  // Simula datos de sensores (puedes reemplazar por analogRead())
  float temp = random(200, 300) / 10.0;  // 20.0 a 30.0
  int hum = random(30, 70);              // 30 a 70%

  // Crea mensaje
  String message = "Temperatura:" + String(temp, 1) + ",Hum:" + String(hum);
  
  // Envía por UART
  Serial1.println(message);
  Serial.println("Enviado -> " + message);

  delay(2000);
}
