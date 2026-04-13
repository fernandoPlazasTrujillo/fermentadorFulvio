/*
 * @file task_actuadores.h
 * @brief Tarea encargada del control de actuadores.
 * 
 * Maneja bomba, motor y servomotor a partir de comandos recibidos.
 * 
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_ACTUADORES_H
#define TASK_ACTUADORES_H

/**
 * @brief Tarea de control de actuadores.
 * 
 * Recibe comandos desde la cola de control y ejecuta acciones
 * sobre bomba, motor y servomotor.
 * 
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_actuadores(void *pvParameters);

#endif