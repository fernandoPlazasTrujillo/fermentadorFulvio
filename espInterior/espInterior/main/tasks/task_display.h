#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

/**
 * @brief Tarea encargada de actualizar la pantalla OLED.
 *
 * Recibe datos desde queue_display y muestra
 * información del proceso de fermentación.
 *
 * @param pvParameters Parámetros de la tarea.
 */
void task_display(void *pvParameters);

#endif