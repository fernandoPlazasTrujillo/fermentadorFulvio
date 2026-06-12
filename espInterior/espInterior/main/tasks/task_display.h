/**
 * @file task_display.h
 * @brief Declaración de la tarea de visualización del sistema.
 *
 * Este módulo define la tarea responsable de actualizar
 * la pantalla OLED con la información más relevante del
 * proceso de fermentación.
 *
 * La tarea recibe datos mediante una cola FreeRTOS y se
 * encarga exclusivamente de la presentación de información,
 * manteniendo desacoplada la interfaz de usuario de las
 * tareas de adquisición, control y comunicación.
 *
 * Información mostrada:
 * - Temperatura.
 * - pH.
 * - Concentración de CO2.
 * - Estado de WiFi.
 * - Estado de MQTT.
 * - Estado de la tarjeta SD.
 * - Fecha y hora del sistema.
 *
 * Arquitectura:
 *
 * queue_display
 *        |
 *        v
 *   task_display
 *        |
 *        v
 *     OLED
 *
 * @author
 * Fernando Plazas Trujillo
 * Isabella Ordoñez
 * Juan Daniel Constain
 */

#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

/**
 * @brief Tarea encargada de actualizar la pantalla OLED.
 *
 * Espera información proveniente de queue_display y
 * actualiza la interfaz de usuario con el estado actual
 * del proceso de fermentación.
 *
 * La tarea no realiza procesamiento de sensores ni toma
 * decisiones de control; únicamente presenta información
 * al usuario.
 *
 * @param pvParameters Parámetros de la tarea (no utilizados).
 */
void task_display(void *pvParameters);

#endif /* TASK_DISPLAY_H */