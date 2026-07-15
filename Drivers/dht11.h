#ifndef DHT11_H
#define DHT11_H
#include "common.h"
void dht11_init(void);
bool dht11_read(int16_t *temperature,uint8_t *humidity);
#endif
