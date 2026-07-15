#ifndef LORA_SERVICE_H
#define LORA_SERVICE_H
#include "common.h"
#include "sx1278.h"
typedef struct { bit enabled; uint32_t frequency_hz; uint8_t sf; uint8_t tx_power; uint16_t period_s; uint16_t network_id; uint16_t node_id; } LoRaConfig;
void lora_service_init(void);
void lora_service_poll(void);
void lora_service_set_enabled(bit enabled);
bit lora_service_configure(const LoRaConfig *cfg);
bit lora_service_test(void);
const LoRaConfig *lora_service_config(void);
#endif
