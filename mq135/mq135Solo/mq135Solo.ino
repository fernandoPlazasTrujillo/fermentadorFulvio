#include "MQ135.h"

// Definir el pin analógico donde se conecta el sensor
const int MQ135_PIN = 4; 
MQ135 gasSensor = MQ135(MQ135_PIN);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Leer el valor del sensor
  float ppm = gasSensor.getPPM();
  
  Serial.print("Calidad del aire (PPM): ");
  Serial.println(ppm);
  
  // Esperar antes de la siguiente lectura
  delay(2000);
}
