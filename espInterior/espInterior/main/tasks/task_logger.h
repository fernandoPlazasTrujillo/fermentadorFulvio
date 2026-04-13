/*
 * @file task_logger.h
 * @brief Tarea de registro de datos en tarjeta SD.
 */

#ifndef TASK_LOGGER_H
#define TASK_LOGGER_H

/**
 * @brief Tarea de logging del sistema.
 * 
 * Recibe datos de sensores, los formatea en CSV y los almacena
 * en la tarjeta SD.
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_logger(void *pvParameters);

#endif