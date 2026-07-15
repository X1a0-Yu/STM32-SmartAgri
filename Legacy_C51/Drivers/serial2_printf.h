#ifndef SERIAL2_PRINTF_H
#define SERIAL2_PRINTF_H

#include "common.h"
#include "app_control.h"

void serial2_init(void);
void serial2_send_char(char value);
void serial2_send_string(const char *text);
void printf_20(const char *fmt, ...);
void serial2_report_all(const SensorData *sensor, const ThresholdConfig *cfg, const OutputState *state);

#endif
