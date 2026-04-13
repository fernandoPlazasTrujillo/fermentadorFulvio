/**
 * @file mq135.h
 * @brief Driver para el sensor de calidad de aire MQ135.
 * 
 * Permite obtener lecturas en bruto (ADC), voltaje y una lectura
 * general compatible con el sistema.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef MQ135_H
#define MQ135_H

/**
 * @brief Inicializa el sensor MQ135.
 * 
 * Actualmente solo registra el estado del sensor.
 */
void mq135_init(void);

/**
 * @brief Lee el valor crudo estimado del ADC.
 * 
 * Convierte el voltaje leído en un valor aproximado de 12 bits (0–4095).
 * 
 * @return Valor ADC estimado.
 */
int mq135_read_raw(void);

/**
 * @brief Lee el voltaje promedio del sensor.
 * 
 * Realiza múltiples lecturas para obtener un valor más estable.
 * 
 * @return Voltaje en voltios.
 */
float mq135_read_voltage(void);

/**
 * @brief Función principal de lectura del sensor.
 * 
 * Diseñada para compatibilidad con el sistema.
 * 
 * @return Valor del sensor (voltaje).
 */
float mq135_read(void);

#endif