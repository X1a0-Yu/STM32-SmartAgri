#ifndef ESP_UART_H
#define ESP_UART_H
#include "common.h"
void esp_uart_init(uint32_t baud);
void esp_uart_set_baud(uint32_t baud);
void esp_uart_putc(char c);
void esp_uart_write(const char *s);
bit esp_uart_get(uint8_t *value);
uint8_t esp_uart_errors(void);
#endif
