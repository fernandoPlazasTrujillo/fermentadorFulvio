#ifndef RTC_DS3231_H
#define RTC_DS3231_H

#include "esp_err.h"
#include "utils/data_types.h"

esp_err_t ds3231_init(void);
esp_err_t ds3231_clear_alarm_flag(void);
esp_err_t ds3231_get_datetime(datetime_t *dt);

esp_err_t ds3231_set_alarm_seconds(uint8_t seconds);

#endif