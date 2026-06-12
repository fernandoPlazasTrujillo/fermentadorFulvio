/**
 * @file task_logger.h
 * @brief Declaración de la tarea de registro de datos.
 *
 * Este módulo define la tarea responsable de almacenar las
 * mediciones generadas durante el proceso de fermentación.
 *
 * La tarea recibe datos provenientes de los sensores mediante
 * una cola FreeRTOS, los formatea adecuadamente y los registra
 * en la tarjeta SD para su posterior análisis.
 *
 * Funciones principales:
 * - Recepción de mediciones del sistema.
 * - Formateo de registros en formato CSV.
 * - Almacenamiento persistente en tarjeta SD.
 * - Conservación del historial de fermentación.
 *
 * Arquitectura:
 *
 * queue_logger
 *        |
 *        v
 *   task_logger
 *        |
 *        v
 *     SD Card
 *
 * Esta tarea permite desacoplar el almacenamiento de datos
 * de las tareas de adquisición y control.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_LOGGER_H
#define TASK_LOGGER_H

/**
 * @brief Tarea principal de registro de datos.
 *
 * Espera información proveniente de queue_logger,
 * genera registros en formato CSV y los almacena
 * en la tarjeta SD.
 *
 * La tarea se ejecuta de forma independiente para evitar
 * que las operaciones de escritura afecten el tiempo de
 * respuesta de las tareas críticas del sistema.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_logger(void *pvParameters);

#endif /* TASK_LOGGER_H */