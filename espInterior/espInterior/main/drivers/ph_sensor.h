/*
 * @file ph_sensor.h
 * @brief Driver para sensor de pH basado en lectura analógica.
 * 
 * Permite obtener el voltaje del sensor y convertirlo a valor de pH.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef PH_SENSOR_H
#define PH_SENSOR_H

/**
 * @brief Inicializa el sensor de pH.
 */
void ph_init(void);

/**
 * @brief Lee el voltaje del sensor de pH.
 * 
 * @return Voltaje promedio en voltios.
 */
float ph_read_voltage(void);

/**
 * @brief Calcula el valor de pH a partir del voltaje.
 * 
 * @return Valor de pH.
 * @return -1.0 si la lectura está fuera de rango.
 */
float ph_read(void);

#endif