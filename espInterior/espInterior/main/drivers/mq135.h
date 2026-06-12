/**
 * @file mq135.h
 * @brief Driver para adquisición de la señal analógica del sensor MQ135.
 *
 * Este módulo permite adquirir la señal de salida del sensor MQ135
 * mediante el ADC del ESP32, obteniendo lecturas en bruto y valores
 * convertidos a voltaje.
 *
 * La conversión a concentraciones específicas de gases (ppm) no se
 * encuentra implementada en este módulo.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef MQ135_H
#define MQ135_H

/**
 * @brief Inicializa el módulo MQ135.
 *
 * Realiza la configuración necesaria para la adquisición
 * de datos desde el canal ADC asociado al sensor.
 *
 * Esta función debe ejecutarse durante la inicialización
 * del sistema antes de realizar lecturas.
 */
void mq135_init(void);

/**
 * @brief Obtiene una lectura cruda del sensor.
 *
 * Realiza una conversión ADC y devuelve el valor digital
 * correspondiente sin aplicar escalamiento adicional.
 *
 * @return Valor digital del ADC.
 */
int mq135_read_raw(void);

/**
 * @brief Obtiene el voltaje promedio de salida del sensor.
 *
 * Se realizan múltiples conversiones ADC y posteriormente
 * se calcula un promedio para reducir el efecto del ruido
 * presente en la señal analógica.
 *
 * @return Voltaje promedio en voltios.
 */
float mq135_read_voltage(void);

/**
 * @brief Obtiene una lectura representativa del sensor MQ135.
 *
 * Esta función actúa como interfaz principal del módulo y
 * proporciona una medición simplificada para su utilización
 * por parte de las tareas del sistema.
 *
 * Actualmente retorna el voltaje promedio de salida del sensor.
 *
 * @return Valor medido por el sensor.
 */
float mq135_read(void);

#endif