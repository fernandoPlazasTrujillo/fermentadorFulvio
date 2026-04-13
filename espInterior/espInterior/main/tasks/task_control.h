/*
 * @file task_control.h
 * @brief Tarea de control del sistema.
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

/**
 * @brief Tarea de lógica de control.
 * 
 * Procesa datos de sensores y genera comandos para actuadores.
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_control(void *pvParameters);

#endif
