#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "app_control.h"

void actuators_init(void);
void actuators_apply(const OutputState *state);

#endif
