#ifndef DISPLAY_H
#define DISPLAY_H

#include "app_control.h"

void display_init(void);
void display_update(const SensorData *sensor, const ThresholdConfig *cfg, UiPage page);

#endif
