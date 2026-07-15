#ifndef ADC0832_H
#define ADC0832_H

#include "common.h"

#define ADC_CHANNEL_AIR 0
#define ADC_CHANNEL_LIGHT 1
#define ADC_CHANNEL_SOIL 0

void adc0832_init(void);
u16 adc0832_read(u8 channel);
u16 sensor_read_air_quality(void);
u16 sensor_read_light(void);
u16 sensor_read_soil(void);

#endif
