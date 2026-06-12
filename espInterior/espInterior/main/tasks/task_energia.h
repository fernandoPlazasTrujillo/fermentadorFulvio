/**
 * @file task_energia.h
 * @brief Declaración de la tarea de gestión energética del sistema.
 *
 * Este módulo define la tarea responsable de coordinar el ciclo
 * operativo del sistema y gestionar los modos de bajo consumo.
 *
 * La tarea supervisa la finalización de actividades críticas
 * del proceso de fermentación y determina cuándo el sistema
 * puede entrar en estados de ahorro energético.
 *
 * Funciones principales:
 * - Coordinación del ciclo de operación.
 * - Sincronización con otras tareas.
 * - Gestión de modos de bajo consumo.
 * - Preparación para entrada en Deep Sleep.
 * - Optimización del consumo energético.
 *
 * Arquitectura:
 *
 * task_actuadores
 *        |
 *        v
 * Notificación FreeRTOS
 *        |
 *        v
 *  task_energia
 *        |
 *        v
 * Deep Sleep
 *
 * Esta tarea forma parte de la estrategia de eficiencia
 * energética del sistema embebido.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_ENERGIA_H
#define TASK_ENERGIA_H

/**
 * @brief Tarea principal de gestión energética.
 *
 * Coordina el ciclo de ejecución del sistema y supervisa
 * la finalización de las tareas necesarias antes de activar
 * mecanismos de ahorro energético.
 *
 * Dependiendo del estado del sistema, puede preparar la
 * transición hacia modos de bajo consumo como Deep Sleep.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_energia(void *pvParameters);

#endif /* TASK_ENERGIA_H */