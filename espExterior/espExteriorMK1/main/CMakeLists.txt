idf_component_register(
    SRCS 
        "main.c"
        "tasks/task_input.c"
        "tasks/task_sensores.c"
        "tasks/task_logger.c"
        "tasks/task_display.c"
        "drivers/dht11.c"
        "drivers/ds3231.c"
        "drivers/sd_card.c"
        "drivers/lcd_parallel.c"

    INCLUDE_DIRS 
        "."
        "include"
        "drivers"
        "tasks"
)