/*
 * Lectura básica MQ-135 en ESP32
 * Modo: depuración (RAW ADC)
 */

#define MQ135_PIN 4

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetPinAttenuation(MQ135_PIN, ADC_11db);

  Serial.println("MQ-135 calibrado (offset)");
}

void loop() {
  int raw = analogRead(MQ135_PIN);

  float voltage_adc = (raw / 4095.0) * 3.3;

  // 🔴 CALIBRACIÓN
  float voltage_real = voltage_adc + 0.12;

  Serial.print("RAW: ");
  Serial.print(raw);

  Serial.print(" | ADC: ");
  Serial.print(voltage_adc, 3);

  Serial.print(" | REAL: ");
  Serial.println(voltage_real, 3);

  delay(1000);
}