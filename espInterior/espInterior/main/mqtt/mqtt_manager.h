#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

void mqtt_manager_start(void);
bool mqtt_wait_connected(uint32_t timeout_ms);
bool mqtt_publish(const char *topic, const char *payload, int qos, uint32_t timeout_ms);
void mqtt_manager_stop(void);
bool mqtt_is_connected(void);

#endif
