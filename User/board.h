#ifndef BOARD_H
#define BOARD_H
#include "board_config.h"
void board_init(void);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool high);
bool gpio_read(GPIO_TypeDef *port, uint8_t pin);
void gpio_config_output(GPIO_TypeDef *port, uint8_t pin, uint8_t mode);
void gpio_config_input(GPIO_TypeDef *port, uint8_t pin, uint8_t mode);
#endif
