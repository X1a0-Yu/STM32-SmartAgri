#ifndef ADC_H
#define ADC_H
#include "common.h"
void adc_init(void);
uint16_t adc_read10(uint8_t channel);
void adc_read_environment(uint16_t *air,uint16_t *light,uint16_t *soil);
#endif
