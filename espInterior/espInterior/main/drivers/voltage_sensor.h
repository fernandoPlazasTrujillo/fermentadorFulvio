/**
 * @file voltage_sensor.h
 * @brief Driver para medición de voltaje mediante conversión ADC.
 *
 * Este módulo permite medir niveles de voltaje utilizando
 * el convertidor analógico-digital (ADC) del ESP32.
 *
 * Debido a las limitaciones de entrada del ADC, la señal
 * es acondicionada mediante un divisor resistivo externo,
 * permitiendo medir tensiones superiores al rango soportado
 * directamente por el microcontrolador.
 *
 * Funcionalidades:
 * - Inicialización del módulo.
 * - Lectura de voltaje compensada.
 * - Conversión a voltaje real del sistema.
 *
 * El módulo es utilizado para monitorear el nivel de batería
 * y apoyar las estrategias de gestión energética del sistema
 * de fermentación.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

/**
 * @brief Inicializa el módulo de medición de voltaje.
 *
 * Prepara los recursos necesarios para realizar
 * conversiones ADC a través de la capa HAL.
 *
 * Esta función debe ejecutarse durante la fase
 * de inicialización del sistema.
 */
void voltage_sensor_init(void);

/**
 * @brief Obtiene el voltaje real medido.
 *
 * Realiza una lectura ADC, aplica las correcciones
 * necesarias y compensa el efecto del divisor
 * resistivo utilizado en la etapa de acondicionamiento.
 *
 * El valor retornado corresponde al voltaje real
 * presente en la fuente o batería monitoreada.
 *
 * @return Voltaje medido en voltios.
 */
float voltage_sensor_read(void);

#endif