/*
 * @file task_energia.h
 * @brief Tarea de gestión de energía del sistema.
 */

#ifndef TASK_ENERGIA_H
#define TASK_ENERGIA_H

/**
 * @brief Tarea de control energético.
 * 
 * Coordina el ciclo de ejecución del sistema y gestiona el
 * paso a modo de bajo consumo (deep sleep).
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_energia(void *pvParameters);

#endif