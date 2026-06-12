/**
 * @file ph_sensor.h
 * @brief Driver para sensor de pH basado en adquisición analógica.
 *
 * Este módulo permite obtener mediciones provenientes de un sensor
 * de pH conectado al ADC del ESP32.
 *
 * Funcionalidades:
 * - Lectura de voltaje.
 * - Conversión de voltaje a unidades de pH.
 * - Validación básica de rangos.
 *
 * El sensor es utilizado para monitorear las condiciones químicas
 * del proceso de fermentación de café.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */
#ifndef PH_SENSOR_H
#define PH_SENSOR_H

/**
 * @brief Inicializa el módulo del sensor de pH.
 *
 * Realiza la configuración necesaria para habilitar
 * la adquisición de señales analógicas asociadas al sensor.
 *
 * Debe ejecutarse una única vez durante la inicialización
 * del sistema.
 */
void ph_init(void);

/**
 * @brief Obtiene el voltaje de salida del sensor.
 *
 * Realiza múltiples conversiones ADC y calcula un valor
 * promedio para reducir el efecto del ruido presente
 * en la señal analógica.
 *
 * @return Voltaje promedio en voltios.
 */
float ph_read_voltage(void);

/**
 * @brief Obtiene una medición de pH.
 *
 * Convierte el voltaje medido por el sensor a una estimación
 * de pH utilizando la ecuación de calibración definida
 * para el sistema.
 *
 * @return Valor estimado de pH.
 * @retval -1.0 Lectura inválida o fuera de rango.
 */
float ph_read(void);

#endif