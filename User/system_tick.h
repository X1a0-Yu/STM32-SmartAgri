#ifndef SYSTEM_TICK_H
#define SYSTEM_TICK_H
#include "common.h"
void system_tick_init(void);
uint32_t system_millis(void);
bool system_due(uint32_t *last, uint32_t period);
void delay_ms(uint32_t ms);
void delay_us(uint16_t us);
uint16_t micros16(void);
#endif
