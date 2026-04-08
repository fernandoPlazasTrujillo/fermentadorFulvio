#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include "esp_adc/adc_oneshot.h"

void adc_manager_init(void);
float adc_manager_read_voltage(adc_channel_t channel);

#endif