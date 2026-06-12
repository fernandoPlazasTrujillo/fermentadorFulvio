/**
 * @file task_sensores.h
 * @brief Declaración de la tarea de adquisición de sensores.
 *
 * Este módulo define la tarea responsable de realizar la
 * adquisición periódica de las variables físicas del sistema
 * de fermentación.
 *
 * La tarea obtiene información desde los sensores conectados
 * al ESP32, realiza el procesamiento inicial de los datos y
 * distribuye los resultados hacia las diferentes tareas del
 * sistema mediante colas FreeRTOS.
 *
 * Sensores gestionados:
 * - DS18B20 (temperatura).
 * - MQ-135 (CO2).
 * - Sensor de pH.
 * - Monitor de voltaje.
 * - Monitor de corriente.
 * - RTC DS3231.
 *
 * Arquitectura:
 *
 *        Sensores
 *            |
 *            v
 *     task_sensores
 *            |
 *     +------+------+------+
 *     |      |      |      |
 *     v      v      v      v
 * logger mqtt display control
 *
 * Esta tarea actúa como productor principal de datos dentro
 * de la arquitectura del sistema.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_SENSORES_H
#define TASK_SENSORES_H

/**
 * @brief Tarea principal de adquisición de sensores.
 *
 * Realiza la lectura de los sensores del sistema,
 * ejecuta el procesamiento inicial de las mediciones
 * y distribuye los datos a través de las colas FreeRTOS
 * correspondientes.
 *
 * Funciones principales:
 * - Lectura de sensores.
 * - Obtención de fecha y hora.
 * - Construcción de estructuras sensor_data_t.
 * - Envío de datos a tareas consumidoras.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_sensores(void *pvParameters);

#endif /* TASK_SENSORES_H */