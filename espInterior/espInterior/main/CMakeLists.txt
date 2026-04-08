idf_component_register(
    SRCS 
        "main.c"

        "tasks/task_sensores.c"
        "tasks/task_control.c"
        "tasks/task_actuadores.c"
        "tasks/task_logger.c"
        "tasks/task_energia.c"

        "drivers/rtc_ds3231.c"
        "drivers/mq135.c"
        "drivers/ph_sensor.c"
        "drivers/ds18b20.c"
        "drivers/servo.c"
        "drivers/sd_card.c"

        "hal/i2c_manager.c"
        "hal/adc_manager.c"

        "utils/queues.c"   # 🔥 CORREGIDO

    INCLUDE_DIRS
        "."
        "tasks"
        "drivers"
        "hal"
        "utils"
)