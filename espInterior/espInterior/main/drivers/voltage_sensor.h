/*
 * @file voltage_sensor.h
 * @brief Módulo para medición de voltaje mediante ADC.
 * 
 * Utiliza un divisor de voltaje para medir niveles superiores
 * al rango del ADC del ESP32.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

/**
 * @brief Inicializa el sensor de voltaje.
 */
void voltage_sensor_init(void);

/**
 * @brief Lee el voltaje real medido.
 * 
 * Aplica compensación de offset y factor de divisor resistivo.
 * 
 * @return Voltaje en voltios.
 */
float voltage_sensor_read(void);

#endif