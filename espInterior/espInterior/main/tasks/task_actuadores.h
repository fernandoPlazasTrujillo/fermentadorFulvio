/**
 * @file task_actuadores.h
 * @brief Declaración de la tarea encargada del control de actuadores.
 *
 * Este módulo define la tarea responsable de ejecutar las acciones
 * físicas del sistema a partir de los comandos generados por la
 * lógica de control.
 *
 * La tarea recibe estructuras control_cmd_t mediante una cola
 * FreeRTOS y actúa sobre los dispositivos de salida del sistema.
 *
 * Actuadores gestionados:
 * - Sistema de enfriamiento.
 * - Bomba de circulación.
 * - Motor de mezcla.
 * - Servomotor.
 *
 * Esta arquitectura mantiene desacopladas las decisiones de control
 * del acceso directo al hardware.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_ACTUADORES_H
#define TASK_ACTUADORES_H

/**
 * @brief Tarea de ejecución de actuadores.
 *
 * Espera comandos provenientes de la cola de control y ejecuta
 * las acciones correspondientes sobre los actuadores físicos
 * del sistema.
 *
 * La tarea opera como consumidora de estructuras control_cmd_t,
 * permitiendo separar la lógica de decisión de la manipulación
 * directa del hardware.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_actuadores(void *pvParameters);

#endif /* TASK_ACTUADORES_H */