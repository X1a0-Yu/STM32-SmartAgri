#ifndef SERIAL2_PRINTF_H
#define SERIAL2_PRINTF_H
#include "app_control.h"
void serial2_init(void);void serial2_putc(char c);void printf_20(const char *fmt,...);bool serial2_rx_get(uint8_t *value);uint8_t serial2_rx_errors(void);void serial2_report_all(const SensorData*,const ThresholdConfig*,const OutputState*);
#endif
