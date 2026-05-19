idf_component_register(
    SRCS 
        "main.c"

        "wifi/wifi_manager.c"
        "mqtt/mqtt_manager.c"

        "tasks/task_sensores.c"
        "tasks/task_logger.c"
        "tasks/task_display.c"
        "tasks/task_mqtt.c"

        "drivers/dht11.c"
        "drivers/ds3231.c"
        "drivers/sd_card.c"
        "drivers/lcd_parallel.c"

    INCLUDE_DIRS 
        "."
        "include"
        "drivers"
        "tasks"
        "wifi"
        "mqtt"
)
