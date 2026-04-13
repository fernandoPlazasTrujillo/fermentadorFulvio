/*
 * @file task_sensores.h
 * @brief Tarea de adquisición de sensores.
 */

#ifndef TASK_SENSORES_H
#define TASK_SENSORES_H

/**
 * @brief Tarea de lectura de sensores.
 * 
 * Obtiene datos de los sensores, aplica procesamiento básico
 * y envía los resultados a las colas del sistema.
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_sensores(void *pvParameters);

#endif