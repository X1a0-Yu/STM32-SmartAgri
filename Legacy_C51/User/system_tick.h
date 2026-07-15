#ifndef SYSTEM_TICK_H
#define SYSTEM_TICK_H

#include "common.h"

void system_tick_init(void);
u32 system_millis(void);
bit system_due(u32 *last_tick, u16 period_ms);
void delay_ms(u16 ms);

#endif
