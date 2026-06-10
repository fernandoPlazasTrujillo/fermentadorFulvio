#include "tasks/task_display.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "drivers/oled_display.h"

#include "queues.h"
#include "utils/data_types.h"

void task_display(void *pvParameters)
{
    display_data_t data;

    char line[32];

    oled_init();
    oled_clear();

    while (1)
    {
        if (xQueueReceive(
                queue_display,
                &data,
                portMAX_DELAY))
        {
            oled_clear();

            // -------------------
            // Temperatura
            // -------------------

            snprintf(
                line,
                sizeof(line),
                "T: %.1f C",
                data.temperatura);

            oled_draw_string(
                0,
                0,
                line);

            // -------------------
            // pH
            // -------------------

            snprintf(
                line,
                sizeof(line),
                "PH: %.2f",
                data.ph);

            oled_draw_string(
                0,
                12,
                line);

            // -------------------
            // CO2
            // -------------------

            snprintf(
                line,
                sizeof(line),
                "CO2: %.2f",
                data.co2);

            oled_draw_string(
                0,
                24,
                line);

            // -------------------
            // MQTT
            // -------------------

            snprintf(
                line,
                sizeof(line),
                "W:%s M:%s S:%s",
                data.wifi_ok ? "OK" : "OFF",
                data.mqtt_ok ? "OK" : "OFF",
                data.sd_ok   ? "OK" : "OFF");

            oled_draw_string(
                0,
                36,
                line);

            // -------------------
            // Hora
            // -------------------

            printf("DISPLAY -> %d:%d:%d\n", data.datetime.hours, data.datetime.minutes, data.datetime.seconds);

            snprintf(
                line,
                sizeof(line),
                "%02d:%02d:%02d",
                data.datetime.hours,
                data.datetime.minutes,
                data.datetime.seconds);

            oled_draw_string(
                0,
                48,
                line);

            oled_update();
        }
    }
}