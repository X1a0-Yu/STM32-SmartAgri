#ifndef BUZZER_H
#define BUZZER_H
#include "common.h"
void buzzer_init(void);
void buzzer_set(bit on);
void buzzer_set_mode2(uint8_t mode);
uint8_t buzzer_get_mode(void);
void beep_stop(void);
#endif
