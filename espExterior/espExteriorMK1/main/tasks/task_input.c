#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "queues.h"

#define BOTON_GPIO GPIO_NUM_4

static TaskHandle_t task_input_handle = NULL;

void IRAM_ATTR boton_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(task_input_handle, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}


void task_input(void *pvParameters)
{
    // 🔴 IMPORTANTE: guardar handle de la tarea
    task_input_handle = xTaskGetCurrentTaskHandle();

    evento_t evento;

    // 🔧 CONFIGURAR GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BOTON_GPIO),
        .pull_up_en = 0,
        .pull_down_en = 0
    };

    gpio_config(&io_conf);

    // 🔧 INSTALAR INTERRUPCIÓN
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BOTON_GPIO, boton_isr_handler, NULL);

    while (1)
    {
        printf("Esperando boton...\n");

        // Espera a que la ISR notifique (evento de botón)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Anti-rebote (debounce básico)
        vTaskDelay(pdMS_TO_TICKS(200));

        // Crear evento
        evento.tipo = EVENTO_INICIO_PROCESO;

        // Enviar a la cola
        xQueueSend(cola_eventos, &evento, portMAX_DELAY);

        printf("Boton presionado -> evento enviado\n");
    }
}