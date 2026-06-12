/**
 * @file task_control.h
 * @brief Declaración de la tarea de control del sistema.
 *
 * Este módulo define la tarea responsable de procesar las
 * mediciones adquiridas por los sensores y generar los
 * comandos necesarios para los actuadores.
 *
 * La tarea implementa la lógica de decisión del sistema,
 * manteniendo separada la capa de control de la capa de
 * acceso al hardware.
 *
 * Flujo de operación:
 *
 * queue_sensores
 *       |
 *       v
 *  task_control
 *       |
 *       v
 * queue_control
 *
 * Esta arquitectura permite desacoplar la adquisición de
 * datos, la toma de decisiones y la ejecución de acciones.
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

/**
 * @brief Tarea principal de control.
 *
 * Recibe datos provenientes de los sensores mediante
 * una cola FreeRTOS, evalúa las condiciones del proceso
 * y genera comandos para los actuadores.
 *
 * La tarea no accede directamente al hardware, sino que
 * envía estructuras control_cmd_t hacia la tarea de
 * actuadores mediante queue_control.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_control(void *pvParameters);

#endif /* TASK_CONTROL_H */