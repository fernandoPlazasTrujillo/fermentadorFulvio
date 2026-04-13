/**
 * @file mq135_read.ino
 * @brief Lectura básica de sensor analógico (pH) usando ESP32.
 * 
 * Este programa realiza la lectura de una señal analógica proveniente
 * de un sensor de pH (o sensor similar como MQ-135 en modo RAW),
 * convirtiendo el valor ADC a voltaje y aplicando una calibración simple.
 * 
 * Los resultados se muestran en el monitor serial para depuración
 * y análisis de datos.
 * 
 * @note El valor de calibración (offset) debe ajustarse según el sensor.
 * @warning Asegúrese de que el voltaje de entrada no exceda 3.3V.
 */

#define MQ135_PIN 4  /**< Pin analógico conectado al sensor */

/**
 * @brief Inicializa la comunicación serial y configura el ADC.
 * 
 * - Inicia la comunicación serial a 115200 baudios.
 * - Configura la resolución del ADC a 12 bits (0–4095).
 * - Ajusta la atenuación del pin para permitir lecturas hasta ~3.3V.
 */
void setup() {
  Serial.begin(115200);

  analogReadResolution(12);                /**< Resolución ADC de 12 bits */
  analogSetPinAttenuation(MQ135_PIN, ADC_11db); /**< Rango de entrada hasta ~3.3V */

  Serial.println("Sensor calibrado (offset aplicado)");
}

/**
 * @brief Lee el valor del sensor y lo convierte a voltaje.
 * 
 * Este ciclo se ejecuta continuamente:
 * - Lee el valor crudo del ADC.
 * - Convierte el valor a voltaje.
 * - Aplica una corrección de calibración (offset).
 * - Imprime los resultados en el monitor serial.
 */
void loop() {
  int raw = analogRead(MQ135_PIN); /**< Valor crudo del ADC (0-4095) */

  float voltage_adc = (raw / 4095.0) * 3.3; /**< Conversión a voltaje ADC */

  /**
   * @brief Corrección de calibración.
   * 
   * Se añade un offset para compensar errores del sensor o del sistema.
   */
  float voltage_real = voltage_adc + 0.12;

  Serial.print("RAW: ");
  Serial.print(raw);

  Serial.print(" | ADC: ");
  Serial.print(voltage_adc, 3);

  Serial.print(" | REAL: ");
  Serial.println(voltage_real, 3);

  delay(1000); /**< Retardo de 1 segundo entre lecturas */
}