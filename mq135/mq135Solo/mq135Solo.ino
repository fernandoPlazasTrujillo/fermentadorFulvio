/**
 * @file mq135_ppm_read.ino
 * @brief Lectura de calidad del aire (PPM) usando sensor MQ-135 y ESP32.
 * 
 * Este programa utiliza la librería MQ135 para obtener directamente
 * una estimación de la concentración de gases en partes por millón (PPM).
 * 
 * El valor calculado se envía al monitor serial para su visualización.
 * 
 * @note La precisión de la medición depende de una correcta calibración del sensor.
 * @warning El sensor MQ-135 requiere tiempo de calentamiento para lecturas estables.
 */

#include "MQ135.h"

// ============================
// CONFIGURACIÓN
// ============================

/**
 * @brief Pin analógico donde está conectado el sensor MQ-135.
 */
const int MQ135_PIN = 4;

/**
 * @brief Objeto para manejar el sensor MQ-135.
 */
MQ135 gasSensor = MQ135(MQ135_PIN);

// ============================
// SETUP
// ============================

/**
 * @brief Inicializa la comunicación serial.
 */
void setup() {
  Serial.begin(115200);
}

// ============================
// LOOP
// ============================

/**
 * @brief Realiza la lectura periódica del sensor MQ-135.
 * 
 * - Obtiene la concentración estimada en PPM.
 * - Imprime el valor en el monitor serial.
 */
void loop() {
  /**
   * @brief Valor de concentración de gases en partes por millón (PPM).
   */
  float ppm = gasSensor.getPPM();

  Serial.print("Calidad del aire (PPM): ");
  Serial.println(ppm);

  /**
   * @brief Retardo entre lecturas.
   */
  delay(2000);
}