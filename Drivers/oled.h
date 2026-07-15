#ifndef OLED_H
#define OLED_H
#include "app_control.h"
typedef UiPage UI_PAGE;
void oled_init(void);
void oled_update(const SensorData*,const ThresholdConfig*,UiPage);
#endif
