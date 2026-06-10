# CMakeLists.txt
# Configuración del componente principal del proyecto ESP-IDF
# Define los archivos fuente y directorios de inclusión

idf_component_register(
    SRCS 
        "main.c"

        "wifi/wifi_manager.c"
        "mqtt/mqtt_manager.c"

        "tasks/task_sensores.c"
        "tasks/task_control.c"
        "tasks/task_actuadores.c"
        "tasks/task_logger.c"
        "tasks/task_energia.c"
        "tasks/task_mqtt.c"
        "tasks/task_display.c"

        "drivers/rtc_ds3231.c"
        "drivers/mq135.c"
        "drivers/ph_sensor.c"
        "drivers/ds18b20.c"
        "drivers/servo.c"
        "drivers/sd_card.c"
        "drivers/oled_display.c"

        "hal/i2c_manager.c"
        "hal/adc_manager.c"

        "utils/queues.c"

    INCLUDE_DIRS
        "."
        "tasks"
        "drivers"
        "hal"
        "utils"
        "wifi"
        "mqtt"
)
