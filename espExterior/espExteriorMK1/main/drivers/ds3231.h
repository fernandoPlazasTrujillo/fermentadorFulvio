/**
 * @file ds3231.h
 * @brief Interfaz pública del controlador para el RTC DS3231.
 *
 * Este módulo proporciona las funciones necesarias para interactuar con
 * el reloj de tiempo real (RTC) DS3231 mediante comunicación I2C.
 *
 * Funcionalidades principales:
 * - Lectura de fecha y hora.
 * - Configuración de fecha y hora.
 * - Configuración de alarmas periódicas.
 * - Gestión de interrupciones generadas por alarmas.
 *
 * @note El acceso al bus I2C debe protegerse mediante mutex cuando sea
 * compartido entre múltiples tareas FreeRTOS.
 *
 * @section rtc_architecture Arquitectura del sistema
 *
 * En el sistema de fermentación de café, el DS3231 actúa como referencia
 * temporal principal para:
 *
 * - Generación de timestamps para almacenamiento en SD.
 * - Sincronización de mediciones de sensores.
 * - Activación periódica mediante interrupciones.
 * - Implementación de estrategias de ahorro energético.
 *
 * @authors
 * - Fernando Plazas
 * - Isabella Ordoñez
 * - Juan Daniel Constain
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

/**
 * @brief Representa una fecha y hora completas.
 *
 * Esta estructura almacena los valores obtenidos desde los registros
 * internos del RTC DS3231 o los valores que serán escritos en él.
 */
typedef struct
{
    /**
     * @brief Segundos.
     *
     * Rango válido: 0 - 59.
     */
    uint8_t segundos;

    /**
     * @brief Minutos.
     *
     * Rango válido: 0 - 59.
     */
    uint8_t minutos;

    /**
     * @brief Horas.
     *
     * Formato de 24 horas.
     * Rango válido: 0 - 23.
     */
    uint8_t horas;

    /**
     * @brief Día del mes.
     *
     * Rango válido: 1 - 31.
     */
    uint8_t dia;

    /**
     * @brief Mes del año.
     *
     * Rango válido: 1 - 12.
     */
    uint8_t mes;

    /**
     * @brief Año completo.
     *
     * Ejemplo: 2026.
     */
    uint16_t anio;

} rtc_time_t;

/**
 * @brief Lee la fecha y hora actuales almacenadas en el RTC.
 *
 * Recupera los registros de tiempo del DS3231 mediante I2C,
 * convirtiendo automáticamente los datos desde formato BCD
 * a formato decimal.
 *
 * @param[out] time Estructura donde se almacenará la fecha y hora.
 *
 * @retval 0 Lectura exitosa.
 * @retval -1 Error de comunicación I2C.
 */
int ds3231_read_time(rtc_time_t *time);

/**
 * @brief Configura la fecha y hora del RTC.
 *
 * Convierte los valores contenidos en la estructura rtc_time_t
 * al formato requerido por el DS3231 y los almacena en sus
 * registros internos.
 *
 * @param[in] time Fecha y hora a configurar.
 *
 * @retval 0 Configuración exitosa.
 * @retval -1 Error de comunicación I2C.
 *
 * @warning Modificar la hora afecta la generación de timestamps,
 * alarmas e historial de datos almacenados por el sistema.
 */
int ds3231_set_time(rtc_time_t *time);

/**
 * @brief Configura una alarma periódica cada minuto.
 *
 * Utiliza la alarma 1 del DS3231 para generar una interrupción
 * exactamente cuando los segundos alcanzan el valor cero.
 *
 * Esta funcionalidad es utilizada para despertar el sistema
 * desde modos de bajo consumo y ejecutar los ciclos de medición.
 *
 * @retval 0 Configuración exitosa.
 * @retval -1 Error de comunicación I2C.
 *
 * @note Requiere que el pin INT/SQW del DS3231 esté conectado
 * a un GPIO configurado como interrupción externa.
 */
int ds3231_set_alarm_every_minute(void);

/**
 * @brief Limpia el indicador interno de alarma.
 *
 * Después de generarse una interrupción, el DS3231 mantiene
 * activo el bit A1F dentro del registro de estado.
 *
 * Esta función limpia dicho bit para permitir que futuras
 * alarmas vuelvan a generar interrupciones.
 *
 * @retval 0 Operación completada.
 *
 * @warning Si no se ejecuta esta función después de atender
 * una alarma, las siguientes interrupciones pueden no producirse.
 */
int ds3231_clear_alarm_flag(void);

#endif