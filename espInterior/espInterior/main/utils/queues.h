#ifndef QUEUES_H
#define QUEUES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "utils/data_types.h"

extern QueueHandle_t queue_sensores;
extern QueueHandle_t queue_control;
extern QueueHandle_t queue_logger;

void queues_init(void);

#endif