#ifndef DHT11_H
#define DHT11_H

#include "common.h"

void dht11_init(void);
bit dht11_read(s16 *temperature, u8 *humidity);

#endif
