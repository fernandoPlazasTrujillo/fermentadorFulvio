#include "queues.h"

QueueHandle_t queue_sensores;
QueueHandle_t queue_control;
QueueHandle_t queue_logger;

void queues_init(void)
{
    queue_sensores = xQueueCreate(5, sizeof(sensor_data_t));
    queue_control  = xQueueCreate(5, sizeof(control_cmd_t));
    queue_logger   = xQueueCreate(5, sizeof(sensor_data_t));
}